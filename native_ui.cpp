#include "native_ui.h"

// Very basic 5x7 font data (simplified ASCII subset)
const uint8_t font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x00, 0x5f, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7f, 0x14, 0x7f, 0x14}, // #
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1c, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1c, 0x00}, // )
    {0x14, 0x08, 0x3e, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3e, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 0
    {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3e}, // @
    {0x7e, 0x11, 0x11, 0x11, 0x7e}, // A
    {0x7f, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3e, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7f, 0x41, 0x41, 0x22, 0x1c}, // D
    {0x7f, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7f, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3e, 0x41, 0x49, 0x49, 0x7a}, // G
    {0x7f, 0x08, 0x08, 0x08, 0x7f}, // H
    {0x00, 0x41, 0x7f, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3f, 0x01}, // J
    {0x7f, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7f, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7f, 0x02, 0x0c, 0x02, 0x7f}, // M
    {0x7f, 0x04, 0x08, 0x10, 0x7f}, // N
    {0x3e, 0x41, 0x41, 0x41, 0x3e}, // O
    {0x7f, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3e, 0x41, 0x51, 0x21, 0x5e}, // Q
    {0x7f, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7f, 0x01, 0x01}, // T
    {0x3f, 0x40, 0x40, 0x40, 0x3f}, // U
    {0x1f, 0x20, 0x40, 0x20, 0x1f}, // V
    {0x3f, 0x40, 0x38, 0x40, 0x3f}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}  // Z
};

NativeUI::NativeUI(uint16_t w, uint16_t h) : _width(w), _height(h), _buffer(nullptr), _font(nullptr) {
    _bufferSize = (size_t)w * h / 8;
}

NativeUI::~NativeUI() {
    if (_buffer) free(_buffer);
}

bool NativeUI::begin() {
    _buffer = (unsigned char*)malloc(_bufferSize);
    if (!_buffer) return false;
    clear(1); // Default to white
    return true;
}

void NativeUI::clear(uint8_t color) {
    memset(_buffer, color ? 0xFF : 0x00, _bufferSize);
}

void NativeUI::drawPixel(int16_t x, int16_t y, uint8_t color) {
    if (x < 0 || y < 0) return;
    uint16_t curW = getWidth();
    uint16_t curH = getHeight();
    if (x >= curW || y >= curH) return;

    int16_t physX, physY;
    switch (_rotation) {
        case 0: physX = x; physY = y; break;
        case 1: physX = y; physY = _height - 1 - x; break;
        case 2: physX = _width - 1 - x; physY = _height - 1 - y; break;
        case 3: physX = _width - 1 - y; physY = x; break;
        default: physX = x; physY = y; break;
    }
    
    // Physical mirroring fix (X flip usually depends on display controller mode)
    // Most Good Display headers for these EPDs need X mirrored for standard buffer
    int16_t mirrorX = _width - 1 - physX;
    
    uint16_t byteIdx = (physY * (_width / 8)) + (mirrorX / 8);
    uint8_t bitIdx = 7 - (mirrorX % 8);
    
    if (color) { // 1 = White
        _buffer[byteIdx] |= (1 << bitIdx);
    } else { // 0 = Black
        _buffer[byteIdx] &= ~(1 << bitIdx);
    }
}

void NativeUI::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t color) {
    for (int16_t i = x; i < x + w; i++) {
        drawPixel(i, y, color);
        drawPixel(i, y + h - 1, color);
    }
    for (int16_t j = y; j < y + h; j++) {
        drawPixel(x, j, color);
        drawPixel(x + w - 1, j, color);
    }
}

void NativeUI::drawChar(int16_t x, int16_t y, char c, uint8_t scale) {
    if (c < 32 || c > 90) return; // Only space to Z for now
    uint8_t idx = c - 32;
    
    for (uint8_t i = 0; i < 5; i++) { // 5 columns
        uint8_t line = font5x7[idx][i];
        for (uint8_t j = 0; j < 8; j++) { // 8 rows
            if (line & (1 << j)) {
                if (scale == 1) {
                    drawPixel(x + i, y + j, 0); // Draw black text
                } else {
                    for (uint8_t sx = 0; sx < scale; sx++) {
                        for (uint8_t sy = 0; sy < scale; sy++) {
                            drawPixel(x + i * scale + sx, y + j * scale + sy, 0);
                        }
                    }
                }
            }
        }
    }
}

void NativeUI::drawText(int16_t x, int16_t y, const char* text, uint8_t scale) {
    int16_t curX = x;
    while (*text) {
        if (*text == '\n') {
            // Basic line break if encountered
            curX = x;
            y += 10 * scale;
        } else {
            drawChar(curX, y, toupper(*text), scale);
            curX += 6 * scale; // 5 columns + 1 space
        }
        text++;
    }
}

uint32_t NativeUI::drawWrappedText(int16_t x, int16_t y, uint16_t maxWidth, const char* text, uint8_t scale) {
    int16_t curX = x;
    int16_t curY = y;
    uint8_t charWidth = 6 * scale;
    uint8_t charHeight = 10 * scale;
    
    char word[64];
    uint8_t wordIdx = 0;
    
    const char* p = text;
    const char* lastGoodP = text;

    while (*p) {
        // Collect a word
        if (*p != ' ' && *p != '\n' && wordIdx < 63) {
            word[wordIdx++] = *p;
        } else {
            word[wordIdx] = '\0';
            
            // Check if word fits on current line
            if (curX + (wordIdx * charWidth) > x + maxWidth) {
                curX = x;
                curY += charHeight;
            }
            
            // Safety break IF we are about to go off bottom
            if (curY > _height - charHeight - 20) { // Keep some margin for footer
                return (uint32_t)(lastGoodP - text);
            }

            // Draw the word
            if (wordIdx > 0) {
                drawText(curX, curY, word, scale);
                curX += wordIdx * charWidth;
            }
            
            // Handle spaces/newlines
            if (*p == ' ') {
                curX += charWidth;
            } else if (*p == '\n') {
                curX = x;
                curY += charHeight;
            }
            
            wordIdx = 0;
            lastGoodP = p + 1; // Update last successfully rendered position
        }
        p++;
    }
    
    // Draw last word if any
    if (wordIdx > 0) {
        if (curX + (wordIdx * charWidth) > x + maxWidth) {
            curX = x;
            curY += charHeight;
        }
        if (curY <= _height - charHeight - 20) {
            word[wordIdx] = '\0';
            drawText(curX, curY, word, scale);
            return (uint32_t)(p - text);
        }
    }
    
    return (uint32_t)(lastGoodP - text);
}

void NativeUI::drawEpdChar(int16_t x, int16_t y, uint32_t cp) {
    if (!_font) return;
    const EpdGlyph* gly = _font->getGlyph(cp);
    if (!gly) return;

    int16_t ox = x + gly->left;
    int16_t oy = y - gly->top; // Papyrix: screenY = baseline - top + row
    
    const uint8_t* bitmap = _font->data->bitmap + gly->dataOffset;
    
    for (int16_t row = 0; row < gly->height; row++) {
        for (int16_t col = 0; col < gly->width; col++) {
            int pixelPos = row * gly->width + col;
            uint8_t byte = bitmap[pixelPos / 4];
            uint8_t bitIdx = (3 - (pixelPos % 4)) * 2;
            uint8_t val = (byte >> bitIdx) & 0x03;
            
            // Papyrix: 0=White, 1=LightGray, 2=DarkGray, 3=Black (font direct)
            // But GfxRenderer does bmpVal = 3 - (byte >> bit_index) & 0x3
            // which means 0=Black, 1=DarkGray, 2=LightGray, 3=White
            // So if val (font direct) is 1, 2, or 3, it's non-white.
            if (val >= 1) { 
                drawPixel(ox + col, oy + row, 0); // Black
            }
        }
    }
}

void NativeUI::drawEpdText(int16_t x, int16_t y, const char* text) {
    if (!_font) return;
    int16_t curX = x;
    const uint8_t* p = (const uint8_t*)text;
    while (*p) {
        uint32_t cp = *p;
        if (cp >= 0x80) {
            // Simplified UTF-8 to Unicode conversion
            if ((cp & 0xE0) == 0xC0) { // 2-byte
                cp = ((cp & 0x1F) << 6) | (p[1] & 0x3F);
                p += 1;
            } else if ((cp & 0xF0) == 0xE0) { // 3-byte
                cp = ((cp & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
                p += 2;
            } else if ((cp & 0xF8) == 0xF0) { // 4-byte
                cp = ((cp & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
                p += 3;
            }
        }

        if (cp == '\n') {
            curX = x;
            y += _font->data->advanceY;
        } else {
            drawEpdChar(curX, y, cp);
            const EpdGlyph* gly = _font->getGlyph(cp);
            if (gly) curX += gly->advanceX;
            else curX += 10;
        }
        p++;
    }
}

uint32_t NativeUI::drawWrappedEpdText(int16_t x, int16_t y, uint16_t maxWidth, const char* text, int lineHeight) {
    if (!_font) return 0;
    if (lineHeight < 0) lineHeight = _font->data->advanceY;
    
    int16_t curX = x;
    int16_t curY = y;
    
    const uint8_t* p = (const uint8_t*)text;
    const uint8_t* lastGoodP = p;
    char word[256];
    uint16_t wordIdx = 0;
    uint16_t wordWidth = 0;

    while (*p) {
        uint32_t cp = *p;
        uint8_t cpLen = 1;
        if (cp >= 0x80) {
            if ((cp & 0xE0) == 0xC0) { cp = ((cp & 0x1F) << 6) | (p[1] & 0x3F); cpLen = 2; }
            else if ((cp & 0xF0) == 0xE0) { cp = ((cp & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F); cpLen = 3; }
            else if ((cp & 0xF8) == 0xF0) { cp = ((cp & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F); cpLen = 4; }
        }

        // Collect a word
        if (cp != ' ' && cp != '\n' && wordIdx < 250) {
            const EpdGlyph* gly = _font->getGlyph(cp);
            if (gly) wordWidth += gly->advanceX;
            else wordWidth += 10;
            for(int i=0; i<cpLen; i++) word[wordIdx++] = p[i];
            p += cpLen;
            continue;
        } else {
            word[wordIdx] = '\0';
            
            if (cp == '\n' && p[1] == '\n') {
                // If we have a pending word, draw it
                if (wordIdx > 0) {
                   uint16_t curW = getWidth();
                   if (curX + wordWidth > x + maxWidth) { curX = x; curY += lineHeight; }
                   drawEpdText(curX, curY, word);
                }
                curX = x;
                curY += lineHeight * 2; // Extra space for paragraph
                wordIdx = 0; wordWidth = 0;
                p += 2; lastGoodP = p;
                
                uint16_t curLogicalH = getHeight();
                if (curY > curLogicalH - 80) return (uint32_t)(lastGoodP - (const uint8_t*)text);
                continue;
            }

            // Word wrap check
            if (curX + wordWidth > x + maxWidth) {
                curX = x;
                curY += lineHeight;
            }
            
            // Safety break for footer
            uint16_t curLogicalH = getHeight();
            if (curY > curLogicalH - 80) { // Stop before the separator line at h-75
                return (uint32_t)(lastGoodP - (const uint8_t*)text);
            }

            // Draw word
            if (wordIdx > 0) {
                drawEpdText(curX, curY, word);
                curX += wordWidth;
            }
            
            // Handle space/newline
            if (*p == ' ') {
                const EpdGlyph* spGly = _font->getGlyph(' ');
                curX += spGly ? spGly->advanceX : 8;
            } else if (*p == '\n') {
                curX = x;
                curY += lineHeight;
            }
            
            wordIdx = 0;
            wordWidth = 0;
            lastGoodP = p + 1;
        }
        p++;
    }
    
    // Last word check
    if (wordIdx > 0) {
        if (curX + wordWidth > x + maxWidth) { curX = x; curY += lineHeight; }
        if (curY <= getHeight() - 45) {
            word[wordIdx] = '\0';
            drawEpdText(curX, curY, word);
            return (uint32_t)(p - (const uint8_t*)text);
        }
    }

    return (uint32_t)(lastGoodP - (const uint8_t*)text);
}

int16_t NativeUI::getEpdTextWidth(const char* text) {
    if (!_font) return 0;
    int w = 0;
    _font->getTextDimensions(text, &w, nullptr);
    return (int16_t)w;
}
