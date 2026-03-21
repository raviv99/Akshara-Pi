#include "EpubExtractor.h"

String EpubExtractor::cleanText(String html) {
    String result = "";
    result.reserve(html.length());
    bool inTag = false;
    for (int i = 0; i < (int)html.length(); i++) {
        if (html[i] == '<') inTag = true;
        else if (html[i] == '>') inTag = false;
        else if (!inTag) result += html[i];
    }
    result.replace("&nbsp;", " ");
    result.replace("&lt;", "<");
    result.replace("&gt;", ">");
    result.replace("&amp;", "&");
    result.replace("&quot;", "\"");
    result.replace("&apos;", "'");
    return result;
}

bool EpubExtractor::extract(String epubPath, String cacheFile) {
    Serial.println("Extracting EPUB: " + epubPath);
    ZipFile zip(epubPath);
    if (!zip.open()) {
        Serial.println("Failed to open ZIP");
        return false;
    }

    // 1. Find OPF path via container.xml
    size_t size;
    uint8_t* containerBuf = zip.readFileToMemory("META-INF/container.xml", &size, true);
    if (!containerBuf) { return false; }
    String container = String((char*)containerBuf);
    free(containerBuf);

    int start = container.indexOf("full-path=\"");
    if (start < 0) { return false; }
    start += 11;
    int end = container.indexOf("\"", start);
    String opfPath = container.substring(start, end);
    String opfBase = "";
    int lastSlash = opfPath.lastIndexOf('/');
    if (lastSlash >= 0) opfBase = opfPath.substring(0, lastSlash + 1);

    // 2. Parse OPF for Spine order
    uint8_t* opfBuf = zip.readFileToMemory(opfPath.c_str(), &size, true);
    if (!opfBuf) { return false; }
    String opf = String((char*)opfBuf);
    free(opfBuf);

    std::vector<String> spineHrefs;
    int pos = opf.indexOf("<spine");
    if (pos < 0) { return false; }
    int spineEnd = opf.indexOf("</spine>", pos);
    
    while (pos < spineEnd) {
        int itemref = opf.indexOf("<itemref", pos);
        if (itemref < 0 || itemref > spineEnd) break;
        int idrefStart = opf.indexOf("idref=\"", itemref) + 7;
        int idrefEnd = opf.indexOf("\"", idrefStart);
        String idref = opf.substring(idrefStart, idrefEnd);
        
        int manifestStart = opf.indexOf("<manifest");
        int idAttrPos = opf.indexOf("id=\"" + idref + "\"", manifestStart);
        if (idAttrPos > 0) {
            int tagStart = opf.lastIndexOf("<item", idAttrPos);
            int tagEnd = opf.indexOf(">", idAttrPos);
            String itemTag = opf.substring(tagStart, tagEnd);
            int hrefAttrStart = itemTag.indexOf("href=\"") + 6;
            int hrefAttrEnd = itemTag.indexOf("\"", hrefAttrStart);
            String href = itemTag.substring(hrefAttrStart, hrefAttrEnd);
            spineHrefs.push_back(opfBase + href);
        }
        pos = idrefEnd;
    }

    // 3. Extract and combine chapters
    File outFile = SD.open(cacheFile, FILE_WRITE);
    if (!outFile) { return false; }

    for (String href : spineHrefs) {
        // Serial.println("  Processing: " + href);
        uint8_t* chapterBuf = zip.readFileToMemory(href.c_str(), &size, true);
        if (chapterBuf) {
            String text = cleanText(String((char *)chapterBuf));
            outFile.println(text);
            free(chapterBuf);
        }
    }
    outFile.close();
    Serial.println("Extraction Complete.");
    return true;
}
