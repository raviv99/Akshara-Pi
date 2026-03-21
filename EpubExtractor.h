#pragma once
#include <Arduino.h>
#include <SD.h>
#include "ZipFile.h"
#include <vector>

class EpubExtractor {
public:
    static bool extract(String epubPath, String cacheFile);

private:
    static String cleanText(String html);
};
