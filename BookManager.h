#pragma once
#include <Arduino.h>
#include <SD.h>
#include <vector>
#include "native_ui.h"

enum ReaderState {
    STATE_LIBRARY,
    STATE_READER
};

class BookManager {
public:
    BookManager(NativeUI& ui);
    void setup();
    void updateList();
    void showFileBrowser(int refreshMode);
    void openBook(String fileName);
    void showCurrentPage();
    
    // Interaction
    void moveUp();
    void moveDown();
    void nextPage();
    void prevPage();
    void select();
    void backToLibrary();

    ReaderState getState() { return currentState; }

private:
    NativeUI& _ui;
    String bookList[10];
    int fileCount = 0;
    int selectedIndex = 0;
    ReaderState currentState = STATE_LIBRARY;
    String currentOpenFile = "";
    
    // Pagination
    uint32_t currentPageOffset = 0;
    uint32_t totalFileSize = 0;
    std::vector<uint32_t> pageHistory; // Stack for "Previous Page" offsets
    String currentBookTitle = "";
};
