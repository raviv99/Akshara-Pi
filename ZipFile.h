#pragma once
#include <SD.h>
#include <string>
#include <unordered_map>
#include <vector>

class ZipFile {
 public:
  struct FileStatSlim {
    uint16_t method;             // Compression method
    uint32_t compressedSize;     // Compressed size
    uint32_t uncompressedSize;   // Uncompressed size
    uint32_t localHeaderOffset;  // Offset of local file header
  };

  struct ZipDetails {
    uint32_t centralDirOffset;
    uint16_t totalEntries;
    bool isSet;
  };

  struct SizeTarget {
    uint64_t hash;
    uint16_t len;
    uint16_t index;
  };

  static uint64_t fnvHash64(const char* s, size_t len) {
    uint64_t hash = 14695981039346656037ull;
    for (size_t i = 0; i < len; i++) {
        hash ^= static_cast<uint8_t>(s[i]);
        hash *= 1099511628211ull;
    }
    return hash;
  }

 private:
  String filePath;
  File file;
  ZipDetails zipDetails = {0, 0, false};
  std::unordered_map<std::string, FileStatSlim> fileStatSlimCache;

  uint32_t lastCentralDirPos = 0;
  bool lastCentralDirPosValid = false;

  bool loadFileStatSlim(const char* filename, FileStatSlim* fileStat);
  long getDataOffset(const FileStatSlim& fileStat);
  bool loadZipDetails();

 public:
  explicit ZipFile(const String& filePath) : filePath(filePath) {}
  ~ZipFile() { if (file) file.close(); }

  bool isOpen() const { return (bool)file; }
  bool open();
  bool close();
  bool loadAllFileStatSlims();
  bool getInflatedFileSize(const char* filename, size_t* size);
  int fillUncompressedSizes(std::vector<SizeTarget>& targets, std::vector<uint32_t>& sizes);
  uint8_t* readFileToMemory(const char* filename, size_t* size = nullptr, bool trailingNullByte = false);
  bool readFileToStream(const char* filename, Print& out, size_t chunkSize = 1024);
};
