#pragma once
#include "EpdFontData.h"

class GlyphCache {
 public:
  static constexpr int CACHE_SIZE = 64;

  GlyphCache() { clear(); }

  void clear() {
    for (int i = 0; i < CACHE_SIZE; i++) {
      entries[i].codepoint = 0xFFFFFFFF;
      entries[i].glyph = nullptr;
    }
  }

  const EpdGlyph* lookup(uint32_t cp) const {
    const int idx = cp % CACHE_SIZE;
    if (entries[idx].codepoint == cp) {
      return entries[idx].glyph;
    }
    return nullptr;
  }

  void store(uint32_t cp, const EpdGlyph* glyph) {
    const int idx = cp % CACHE_SIZE;
    entries[idx].codepoint = cp;
    entries[idx].glyph = glyph;
  }

 private:
  struct CacheEntry {
    uint32_t codepoint;
    const EpdGlyph* glyph;
  };
  CacheEntry entries[CACHE_SIZE];
};

class EpdFont {
 public:
  const EpdFontData* data;
  explicit EpdFont(const EpdFontData* data) : data(data) {}
  
  const EpdGlyph* getGlyph(uint32_t cp) const;
  void getTextDimensions(const char* string, int* w, int* h) const;

 private:
  mutable GlyphCache glyphCache;
};
