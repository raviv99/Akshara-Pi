#include "EpdFont.h"

const EpdGlyph* EpdFont::getGlyph(uint32_t cp) const {
  if (const EpdGlyph* cached = glyphCache.lookup(cp)) {
    return cached;
  }

  for (uint32_t i = 0; i < data->intervalCount; i++) {
    const EpdUnicodeInterval& interval = data->intervals[i];
    if (cp >= interval.first && cp <= interval.last) {
      const EpdGlyph* gly = &data->glyph[interval.offset + (cp - interval.first)];
      glyphCache.store(cp, gly);
      return gly;
    }
  }
  return nullptr;
}

void EpdFont::getTextDimensions(const char* string, int* w, int* h) const {
  int totalWidth = 0;
  int maxHeight = 0;
  
  const char* p = string;
  while (*p) {
    uint32_t cp = *p;
    uint8_t cpLen = 1;
    if (cp >= 0x80) {
        if ((cp & 0xE0) == 0xC0) { cp = ((cp & 0x1F) << 6) | (p[1] & 0x3F); cpLen = 2; }
        else if ((cp & 0xF0) == 0xE0) { cp = ((cp & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F); cpLen = 3; }
        else if ((cp & 0xF8) == 0xF0) { cp = ((cp & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F); cpLen = 4; }
    }
    
    const EpdGlyph* gly = getGlyph(cp);
    if (gly) {
      totalWidth += gly->advanceX;
      if (gly->height > maxHeight) maxHeight = gly->height;
    }
    p += cpLen;
  }
  
  if (w) *w = totalWidth;
  if (h) *h = maxHeight;
}
