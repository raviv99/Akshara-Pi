#include "BookManager.h"
#include "EpubExtractor.h"
#include "Display_EPD_W21.h"

BookManager::BookManager(NativeUI& ui) : _ui(ui) {}

void BookManager::setup() {
    updateList();
}

void BookManager::updateList() {
    fileCount = 0;
    Serial.println("Scanning SD root for books...");
    File root = SD.open("/");
    if (!root) {
        Serial.println("ERROR: Failed to open SD root!");
        return;
    }

    while (File entry = root.openNextFile()) {
        if (!entry.isDirectory()) {
            String name = entry.name();
            String lowerName = name;
            lowerName.toLowerCase();
            if (lowerName.endsWith(".epub") || lowerName.endsWith(".txt")) {
                Serial.printf("  Found: %s\n", name.c_str());
                bookList[fileCount++] = name;
            }
        }
        entry.close();
        if (fileCount >= 10) break;
    }
    root.close();
    Serial.printf("Scan complete. Total books: %d\n", fileCount);
}

void BookManager::showFileBrowser(int refreshMode) {
    _ui.clear(1);
    uint16_t w = _ui.getWidth();
    uint16_t h = _ui.getHeight();
    _ui.drawRect(0, 0, w, 80, 0);
    _ui.drawEpdText(20, 50, "MY LIBRARY");
    
    if (fileCount == 0) {
        _ui.drawText(40, 150, "NO BOOKS FOUND IN ROOT", 2);
    } else {
        int y = 100;
        for (int i = 0; i < fileCount; i++) {
            if (i == selectedIndex) {
                 _ui.drawRect(20, y - 5, w - 40, 35, 0);
            }
            char fileInfo[100];
            snprintf(fileInfo, sizeof(fileInfo), "  %s", bookList[i].c_str());
            _ui.drawEpdText(40, y + 10, fileInfo); // Adjust for baseline
            y += 40;
        }
    }

    if (refreshMode == 2) { // Full
        EPD_init();
        PIC_display(_ui.getBuffer());
        EPD_sleep();
    } else if (refreshMode == 1) { // Partial
        EPD_init_Part();
        PIC_display_Part_ALL(_ui.getBuffer(), _ui.getBuffer());
        EPD_sleep();
    }
}

void BookManager::openBook(String fileName) {
    String originalPath = "/" + fileName;
    String lowerName = fileName;
    lowerName.toLowerCase();
    String finalPath = originalPath;
    
    if (lowerName.endsWith(".epub")) {
        if (!SD.exists("/.epub_cache")) SD.mkdir("/.epub_cache");
        String cacheName = fileName;
        cacheName.replace(" ", "_");
        String cachePath = "/.epub_cache/" + cacheName + ".txt";
        
        if (!SD.exists(cachePath)) {
            _ui.clear(1);
            uint16_t w = _ui.getWidth();
            uint16_t h = _ui.getHeight();
            _ui.drawRect(w/2 - 260, h/2 - 75, 520, 150, 0);
            _ui.drawText(w/2 - 210, h/2 - 45, "EXTRACTING EPUB...", 2);
            _ui.drawText(w/2 - 230, h/2 + 5, "ONE-TIME PROCESS - PLEASE WAIT", 1);
            EPD_init();
            PIC_display(_ui.getBuffer());
            EPD_sleep();
            
            if (!EpubExtractor::extract(originalPath, cachePath)) {
                finalPath = "";
            } else {
                finalPath = cachePath;
            }
        } else {
            finalPath = cachePath;
        }
    }

    if (finalPath == "") {
        _ui.clear(1);
        _ui.drawText(40, 150, "FAILED TO OPEN BOOK", 2);
        EPD_init();
        PIC_display(_ui.getBuffer());
        EPD_sleep();
        return;
    }

    currentOpenFile = finalPath;
    currentBookTitle = fileName;
    currentPageOffset = 0;
    
    File f = SD.open(finalPath);
    if (f) {
        totalFileSize = f.size();
        f.close();
    } else {
        totalFileSize = 0;
    }

    pageHistory.clear();
    currentState = STATE_READER;
    
    showCurrentPage();
}

void BookManager::showCurrentPage() {
    _ui.clear(1);
    uint16_t w = _ui.getWidth();
    uint16_t h = _ui.getHeight();
    _ui.drawRect(0, 0, w, 50, 0);
    _ui.drawEpdText(20, 35, currentBookTitle.c_str());
    
    File f = SD.open(currentOpenFile);
    if (f) {
        if (currentPageOffset > 0) f.seek(currentPageOffset);
        
        char* buffer = (char*)malloc(4097);
        if (!buffer) {
            _ui.drawText(40, 150, "MEMORY ALLOCATION FAILED", 2);
            f.close();
            return;
        }

        int bytesRead = f.read((uint8_t*)buffer, 4096);
        buffer[bytesRead] = '\0';
        
        // Process the buffer for any control characters if needed, but keep UTF-8 intact
        buffer[bytesRead] = '\0';
        String content = String(buffer);
        
        uint32_t consumed = _ui.drawWrappedEpdText(60, 100, w - 120, content.c_str()); 
        
        // Progress Calculation - use long long for calculation to avoid overflow
        // We calculate based on the offset of the NEXT page (current offset + consumed)
        // to show "progress including this page" or just stick to current offset.
        // Let's use current offset for consistency with "Start of page X".
        int currentPage = (int)pageHistory.size() + 1;
        int estimatedTotalPages = (int)(totalFileSize / 1800) + 1; // Slightly more per page for Literata
        if (currentPage > estimatedTotalPages) estimatedTotalPages = currentPage;
        
        int percent = 0;
        if (totalFileSize > 0) {
            percent = (int)((unsigned long long)currentPageOffset * 100 / totalFileSize);
        }
        if (percent > 100) percent = 100;

        char footer[128];
        snprintf(footer, sizeof(footer), "Page %d of %d  |  %d%%", 
                 currentPage, estimatedTotalPages, percent);
        
        int16_t footerW = _ui.getEpdTextWidth(footer);
        int16_t footerX = (w - footerW) / 2;
        
        // Draw separator line above footer (Margin h-75 for safety)
        for(int16_t lx = 40; lx < w - 40; lx++) _ui.drawPixel(lx, h - 75, 0);
        
        // Draw footer text at h - 15 (bottom edge safety)
        _ui.drawEpdText(footerX, h - 15, footer);
        
        free(buffer);
        f.close();
    } else {
        _ui.drawText(40, 150, "FAILED TO READ FILE", 2);
    }
    
    EPD_init();
    PIC_display(_ui.getBuffer());
    EPD_sleep();
}

void BookManager::nextPage() {
    File f = SD.open(currentOpenFile);
    if (!f) return;
    
    if (currentPageOffset > 0) f.seek(currentPageOffset);
    char* buffer = (char*)malloc(4097);
    if (!buffer) { f.close(); return; }

    int bytesRead = f.read((uint8_t*)buffer, 4096);
    buffer[bytesRead] = '\0';
    String content = "";
    for (int i = 0; i < bytesRead; i++) {
        if (isprint(buffer[i]) || isspace(buffer[i]) || buffer[i] == '\n') {
            content += buffer[i];
        }
    }
    f.close();

    _ui.clear(1); // Invisible render
    uint32_t stringConsumed = _ui.drawWrappedEpdText(60, 100, _ui.getWidth() - 120, content.c_str());
    
    if (stringConsumed > 0) {
        pageHistory.push_back(currentPageOffset);
        currentPageOffset += stringConsumed;
        free(buffer);
        showCurrentPage();
    } else {
        free(buffer);
    }
}

void BookManager::prevPage() {
    if (!pageHistory.empty()) {
        currentPageOffset = pageHistory.back();
        pageHistory.pop_back();
        showCurrentPage();
    }
}

void BookManager::moveUp() {
    if (fileCount > 0) {
        selectedIndex = (selectedIndex - 1 + fileCount) % fileCount;
        showFileBrowser(1);
    }
}

void BookManager::moveDown() {
    if (fileCount > 0) {
        selectedIndex = (selectedIndex + 1) % fileCount;
        showFileBrowser(1);
    }
}

void BookManager::select() {
    if (currentState == STATE_LIBRARY && fileCount > 0) {
        openBook(bookList[selectedIndex]);
    } else if (currentState == STATE_READER) {
        backToLibrary();
    }
}

void BookManager::backToLibrary() {
    currentState = STATE_LIBRARY;
    showFileBrowser(2);
}
