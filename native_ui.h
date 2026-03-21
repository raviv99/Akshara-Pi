#ifndef NATIVE_UI_H
#define NATIVE_UI_H

#include <Arduino.h>
#include "EpdFont.h"

class NativeUI {
public:
    NativeUI(uint16_t w, uint16_t h);
    ~NativeUI();

    bool begin();
    void setRotation(uint8_t r) { _rotation = r % 4; }
    uint8_t getRotation() { return _rotation; }
    uint16_t getWidth() { return (_rotation % 2 == 0) ? _width : _height; }
    uint16_t getHeight() { return (_rotation % 2 == 0) ? _height : _width; }

    void clear(uint8_t color); // 0 = Black, 1 = White
    void drawPixel(int16_t x, int16_t y, uint8_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color);
    void drawText(int16_t x, int16_t y, const char* text, uint8_t scale = 1);
    uint32_t drawWrappedText(int16_t x, int16_t y, uint16_t maxWidth, const char* text, uint8_t scale = 1);
    
    // New EpdFont methods
    void setFont(EpdFont* font) { _font = font; }
    void drawEpdChar(int16_t x, int16_t y, uint32_t cp);
    void drawEpdText(int16_t x, int16_t y, const char* text);
    uint32_t drawWrappedEpdText(int16_t x, int16_t y, uint16_t maxWidth, const char* text, int lineHeight = -1);
    int16_t getEpdTextWidth(const char* text);
    
    unsigned char* getBuffer() { return _buffer; }
    size_t getBufferSize() { return _bufferSize; }

private:
    uint16_t _width; // Physical width (920)
    uint16_t _height; // Physical height (680)
    uint8_t _rotation = 0;
    unsigned char* _buffer;
    size_t _bufferSize;
    EpdFont* _font;
    
    void drawChar(int16_t x, int16_t y, char c, uint8_t scale);
};

#endif
