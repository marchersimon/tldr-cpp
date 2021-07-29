#pragma once

#include <curl/curl.h>
#include <zip.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include "global.h"
#include "opts.h"

struct zipEntry {
    std::string path;
    bool isFile = false;
    bool wanted;
    std::string language;
    std::string platform;
    std::string filename;
};

void parseEntry(struct zipEntry &entry);

int libcurl_ProgressCallback(void* ptr, double dltotal, double dlnow, double utotal, double ulnow);

size_t libcurl_WriteCallback(char* buffer, size_t itemsize, size_t nitems, void* tldrZipPtr);

void downloadZip(string path);

void printDiff();

void updateCache();