#include "ZipFile.h"
#include "miniz.h"
#include <algorithm>

bool inflateOneShot(const uint8_t* inputBuf, const size_t deflatedSize, uint8_t* outputBuf, const size_t inflatedSize) {
  tinfl_decompressor* inflator = (tinfl_decompressor*)malloc(sizeof(tinfl_decompressor));
  if (!inflator) return false;
  tinfl_init(inflator);

  size_t inBytes = deflatedSize;
  size_t outBytes = inflatedSize;
  const tinfl_status status = tinfl_decompress(inflator, inputBuf, &inBytes, nullptr, outputBuf, &outBytes,
                                               TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);

  free(inflator);
  if (status != TINFL_STATUS_DONE) {
    Serial.printf("tinfl_decompress() failed with status %d\n", status);
    return false;
  }

  return true;
}

bool ZipFile::loadAllFileStatSlims() {
  const bool wasOpen = isOpen();
  if (!wasOpen && !open()) {
    return false;
  }

  if (!loadZipDetails()) {
    if (!wasOpen) close();
    return false;
  }

  file.seek(zipDetails.centralDirOffset);

  uint32_t sig;
  char itemName[256];
  fileStatSlimCache.clear();
  // fileStatSlimCache.reserve(zipDetails.totalEntries); // std::unordered_map::reserve() not in all compilers

  while (file.available()) {
    file.read((uint8_t*)&sig, 4);
    if (sig != 0x02014b50) break;

    FileStatSlim fileStat = {};
    file.seek(file.position() + 6);
    file.read((uint8_t*)&fileStat.method, 2);
    file.seek(file.position() + 8);
    file.read((uint8_t*)&fileStat.compressedSize, 4);
    file.read((uint8_t*)&fileStat.uncompressedSize, 4);
    uint16_t nameLen, m, k;
    file.read((uint8_t*)&nameLen, 2);
    file.read((uint8_t*)&m, 2);
    file.read((uint8_t*)&k, 2);
    file.seek(file.position() + 8);
    file.read((uint8_t*)&fileStat.localHeaderOffset, 4);
    
    if (nameLen < 256) {
        file.read((uint8_t*)itemName, nameLen);
        itemName[nameLen] = '\0';
        fileStatSlimCache[std::string(itemName)] = fileStat;
    } else {
        file.seek(file.position() + nameLen);
    }

    file.seek(file.position() + m + k);
  }

  lastCentralDirPos = zipDetails.centralDirOffset;
  lastCentralDirPosValid = true;

  if (!wasOpen) close();
  return true;
}

bool ZipFile::loadFileStatSlim(const char* filename, FileStatSlim* fileStat) {
  if (!fileStatSlimCache.empty()) {
    const auto it = fileStatSlimCache.find(filename);
    if (it != fileStatSlimCache.end()) {
      *fileStat = it->second;
      return true;
    }
  }

  const bool wasOpen = isOpen();
  if (!wasOpen && !open()) return false;

  if (!loadZipDetails()) {
    if (!wasOpen) close();
    return false;
  }

  uint32_t startPos = lastCentralDirPosValid ? lastCentralDirPos : zipDetails.centralDirOffset;
  bool wrapped = false;
  bool found = false;

  file.seek(startPos);

  uint32_t sig;
  char itemName[256];

  while (true) {
    uint32_t entryStart = file.position();

    if (file.read((uint8_t*)&sig, 4) != 4 || sig != 0x02014b50) {
      if (!wrapped && lastCentralDirPosValid && startPos != zipDetails.centralDirOffset) {
        file.seek(zipDetails.centralDirOffset);
        wrapped = true;
        continue;
      }
      break;
    }

    if (wrapped && entryStart >= startPos) break;

    file.seek(file.position() + 6);
    file.read((uint8_t*)&fileStat->method, 2);
    file.seek(file.position() + 8);
    file.read((uint8_t*)&fileStat->compressedSize, 4);
    file.read((uint8_t*)&fileStat->uncompressedSize, 4);
    uint16_t nameLen, m, k;
    file.read((uint8_t*)&nameLen, 2);
    file.read((uint8_t*)&m, 2);
    file.read((uint8_t*)&k, 2);
    file.seek(file.position() + 8);
    file.read((uint8_t*)&fileStat->localHeaderOffset, 4);

    if (nameLen < 256) {
      file.read((uint8_t*)itemName, nameLen);
      itemName[nameLen] = '\0';

      if (strcmp(itemName, filename) == 0) {
        file.seek(file.position() + m + k);
        lastCentralDirPos = file.position();
        lastCentralDirPosValid = true;
        found = true;
        break;
      }
    } else {
      file.seek(file.position() + nameLen);
    }

    file.seek(file.position() + m + k);
  }

  if (!wasOpen) close();
  return found;
}

long ZipFile::getDataOffset(const FileStatSlim& fileStat) {
  const bool wasOpen = isOpen();
  if (!wasOpen && !open()) return -1;

  constexpr auto localHeaderSize = 30;
  uint8_t pLocalHeader[localHeaderSize];
  const uint32_t fileOffset = fileStat.localHeaderOffset;

  file.seek(fileOffset);
  const size_t bytesRead = file.read(pLocalHeader, localHeaderSize);
  
  if (!wasOpen) close();

  if (bytesRead != localHeaderSize) {
    Serial.println("Error reading local header");
    return -1;
  }

  uint32_t sig = pLocalHeader[0] | (pLocalHeader[1] << 8) | (pLocalHeader[2] << 16) | (pLocalHeader[3] << 24);
  if (sig != 0x04034b50) {
    Serial.println("Not a valid local header signature");
    return -1;
  }

  const uint16_t filenameLength = pLocalHeader[26] | (pLocalHeader[27] << 8);
  const uint16_t extraOffset = pLocalHeader[28] | (pLocalHeader[29] << 8);
  return fileOffset + localHeaderSize + filenameLength + extraOffset;
}

bool ZipFile::loadZipDetails() {
  if (zipDetails.isSet) return true;

  const bool wasOpen = isOpen();
  if (!wasOpen && !open()) return false;

  const size_t fileSize = file.size();
  if (fileSize < 22) {
    if (!wasOpen) close();
    return false;
  }

  const int scanRange = fileSize > 1024 ? 1024 : fileSize;
  uint8_t* buffer = (uint8_t*)malloc(scanRange);
  if (!buffer) {
    if (!wasOpen) close();
    return false;
  }

  file.seek(fileSize - scanRange);
  file.read(buffer, scanRange);

  int foundOffset = -1;
  for (int i = scanRange - 22; i >= 0; i--) {
    if (buffer[i] == 0x50 && buffer[i+1] == 0x4b && buffer[i+2] == 0x05 && buffer[i+3] == 0x06) {
      foundOffset = i;
      break;
    }
  }

  if (foundOffset == -1) {
    free(buffer);
    if (!wasOpen) close();
    return false;
  }

  zipDetails.totalEntries = buffer[foundOffset + 10] | (buffer[foundOffset + 11] << 8);
  zipDetails.centralDirOffset = buffer[foundOffset + 16] | (buffer[foundOffset + 17] << 8) | (buffer[foundOffset + 18] << 16) | (buffer[foundOffset + 19] << 24);
  zipDetails.isSet = true;

  free(buffer);
  if (!wasOpen) close();
  return true;
}

bool ZipFile::open() {
  if (isOpen()) return true;
  file = SD.open(filePath);
  return (bool)file;
}

bool ZipFile::close() {
  if (file) {
    file.close();
  }
  lastCentralDirPos = 0;
  lastCentralDirPosValid = false;
  return true;
}

bool ZipFile::getInflatedFileSize(const char* filename, size_t* size) {
  FileStatSlim fileStat = {};
  if (!loadFileStatSlim(filename, &fileStat)) return false;
  *size = (size_t)fileStat.uncompressedSize;
  return true;
}

int ZipFile::fillUncompressedSizes(std::vector<SizeTarget>& targets, std::vector<uint32_t>& sizes) {
  if (targets.empty()) return 0;
  const bool wasOpen = isOpen();
  if (!wasOpen && !open()) return 0;
  if (!loadZipDetails()) {
    if (!wasOpen) close();
    return 0;
  }

  file.seek(zipDetails.centralDirOffset);
  int matched = 0;
  uint32_t sig;
  char itemName[256];

  while (file.available()) {
    if (file.read((uint8_t*)&sig, 4) != 4 || sig != 0x02014b50) break;

    file.seek(file.position() + 20); // skip to name info
    uint32_t uncompressedSize;
    file.read((uint8_t*)&uncompressedSize, 4);
    uint16_t nameLen, m, k;
    file.read((uint8_t*)&nameLen, 2);
    file.read((uint8_t*)&m, 2);
    file.read((uint8_t*)&k, 2);
    file.seek(file.position() + 8);
    
    if (nameLen < 256) {
      file.read((uint8_t*)itemName, nameLen);
      itemName[nameLen] = '\0';

      uint64_t hash = fnvHash64(itemName, nameLen);
      SizeTarget key = {hash, nameLen, 0};

      auto it = std::lower_bound(targets.begin(), targets.end(), key, [](const SizeTarget& a, const SizeTarget& b) {
        return a.hash < b.hash || (a.hash == b.hash && a.len < b.len);
      });

      while (it != targets.end() && it->hash == hash && it->len == nameLen) {
        if (it->index < sizes.size()) {
          sizes[it->index] = uncompressedSize;
          matched++;
        }
        ++it;
      }
    } else {
      file.seek(file.position() + nameLen);
    }
    file.seek(file.position() + m + k);
  }

  if (!wasOpen) close();
  return matched;
}

uint8_t* ZipFile::readFileToMemory(const char* filename, size_t* size, const bool trailingNullByte) {
  const bool wasOpen = isOpen();
  if (!wasOpen && !open()) return nullptr;

  FileStatSlim fileStat = {};
  if (!loadFileStatSlim(filename, &fileStat)) {
    if (!wasOpen) close();
    return nullptr;
  }

  const long fileOffset = getDataOffset(fileStat);
  if (fileOffset < 0) {
    if (!wasOpen) close();
    return nullptr;
  }

  file.seek(fileOffset);

  const uint32_t deflatedDataSize = fileStat.compressedSize;
  const uint32_t inflatedDataSize = fileStat.uncompressedSize;
  const uint32_t dataSize = trailingNullByte ? inflatedDataSize + 1 : inflatedDataSize;
  
  uint8_t* data = (uint8_t*)malloc(dataSize);
  if (!data) {
    if (!wasOpen) close();
    return nullptr;
  }

  if (fileStat.method == 0) { // No Compression
    file.read(data, inflatedDataSize);
  } else if (fileStat.method == 8) { // Deflate
    uint8_t* deflatedData = (uint8_t*)malloc(deflatedDataSize);
    if (!deflatedData) {
      free(data);
      if (!wasOpen) close();
      return nullptr;
    }
    file.read(deflatedData, deflatedDataSize);
    if (!inflateOneShot(deflatedData, deflatedDataSize, data, inflatedDataSize)) {
      free(deflatedData);
      free(data);
      if (!wasOpen) close();
      return nullptr;
    }
    free(deflatedData);
  } else {
    free(data);
    if (!wasOpen) close();
    return nullptr;
  }

  if (trailingNullByte) data[inflatedDataSize] = '\0';
  if (size) *size = inflatedDataSize;
  if (!wasOpen) close();
  return data;
}

bool ZipFile::readFileToStream(const char* filename, Print& out, const size_t chunkSize) {
  const bool wasOpen = isOpen();
  if (!wasOpen && !open()) return false;

  FileStatSlim fileStat = {};
  if (!loadFileStatSlim(filename, &fileStat)) {
    if (!wasOpen) close();
    return false;
  }

  const long fileOffset = getDataOffset(fileStat);
  if (fileOffset < 0) {
    if (!wasOpen) close();
    return false;
  }

  file.seek(fileOffset);
  const uint32_t deflatedDataSize = fileStat.compressedSize;
  const uint32_t inflatedDataSize = fileStat.uncompressedSize;

  if (fileStat.method == 0) {
    uint8_t* buffer = (uint8_t*)malloc(chunkSize);
    if (!buffer) { if (!wasOpen) close(); return false; }

    size_t remaining = inflatedDataSize;
    while (remaining > 0) {
      size_t toRead = remaining < chunkSize ? remaining : chunkSize;
      size_t bytesRead = file.read(buffer, toRead);
      if (bytesRead == 0) break;
      out.write(buffer, bytesRead);
      remaining -= bytesRead;
    }
    free(buffer);
    if (!wasOpen) close();
    return true;
  }

  if (fileStat.method == 8) {
    tinfl_decompressor* inflator = (tinfl_decompressor*)malloc(sizeof(tinfl_decompressor));
    if (!inflator) { if (!wasOpen) close(); return false; }
    tinfl_init(inflator);

    uint8_t* fileReadBuffer = (uint8_t*)malloc(chunkSize);
    uint8_t* outputBuffer = (uint8_t*)malloc(TINFL_LZ_DICT_SIZE);
    
    if (!fileReadBuffer || !outputBuffer) {
        if (fileReadBuffer) free(fileReadBuffer);
        if (outputBuffer) free(outputBuffer);
        free(inflator);
        if (!wasOpen) close();
        return false;
    }

    size_t fileRemainingBytes = deflatedDataSize;
    size_t fileReadBufferFilledBytes = 0;
    size_t fileReadBufferCursor = 0;
    size_t outputCursor = 0;

    bool result = true;
    while (true) {
      if (fileReadBufferCursor >= fileReadBufferFilledBytes) {
        if (fileRemainingBytes == 0) break;
        fileReadBufferFilledBytes = file.read(fileReadBuffer, fileRemainingBytes < chunkSize ? fileRemainingBytes : chunkSize);
        fileRemainingBytes -= fileReadBufferFilledBytes;
        fileReadBufferCursor = 0;
        if (fileReadBufferFilledBytes == 0) break;
      }

      size_t inBytes = fileReadBufferFilledBytes - fileReadBufferCursor;
      size_t outBytes = TINFL_LZ_DICT_SIZE - outputCursor;

      tinfl_status status = tinfl_decompress(inflator, fileReadBuffer + fileReadBufferCursor, &inBytes,
                                             outputBuffer, outputBuffer + outputCursor, &outBytes,
                                             fileRemainingBytes > 0 ? TINFL_FLAG_HAS_MORE_INPUT : 0);

      fileReadBufferCursor += inBytes;
      if (outBytes > 0) {
        out.write(outputBuffer + outputCursor, outBytes);
        outputCursor = (outputCursor + outBytes) & (TINFL_LZ_DICT_SIZE - 1);
      }

      if (status < 0) { result = false; break; }
      if (status == TINFL_STATUS_DONE) break;
    }

    free(outputBuffer);
    free(fileReadBuffer);
    free(inflator);
    if (!wasOpen) close();
    return result;
  }

  if (!wasOpen) close();
  return false;
}
