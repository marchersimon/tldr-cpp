#pragma once

#include <curl/curl.h>
#include <zip.h>
#include <fstream>
#include <filesystem>
#include "global.h"

struct zipEntry {
    std::string path;
    bool isFile = false;
    bool wanted;
    std::string language;
    std::string platform;
    std::string filename;
};

bool isPage(const char* path) {
    if(std::string(path).find(".md") != -1) {
        return true;
    }
    return false;

}

/*
Parses the path to an entry in the zip file. It extracts the language code, platform and file name. 
The path can be pages(.xyz)/, pages(.xyz)/platform/ or pages(.xyz)/platform/filename.md
*/
void parseEntry(struct zipEntry &entry) {

    entry.isFile = false;
    entry.wanted = false;
    entry.language = "";
    entry.platform = "";
    entry.filename = "";

    int start = 0;
    int end = 0;
    int i = 0;

    if(entry.path == "index.json") {
        entry.wanted = false;
        return;
    }
    if(entry.path == "LICENSE.md") {
        entry.wanted = true;
        entry.isFile = true;
        return;
    }

    do {
        end = entry.path.find('/', start);
        switch(i) {
            case 0:
                if(end - start == 5) {
                    entry.language = "en";
                } else {
                    entry.language = entry.path.substr(start + 6, end - 6);
                }
                break;
            case 1:
                entry.platform = entry.path.substr(start, end - start);
                break;
            case 2: 
                entry.filename = entry.path.substr(start, end - start);
                if(entry.filename != "") {
                    entry.isFile = true;
                }
                break;
        }
        start = end + 1;
        i++;
    } while(end != std::string::npos);

    // only extract pages in a specific language
    //if(entry.language == "de") entry.wanted = true;
    entry.wanted = true;
}

size_t processChunk(char* buffer, size_t itemsize, size_t nitems, void* tldrZip) {
    size_t bytes = itemsize * nitems;
    std::ofstream* tldrZip1 = (std::ofstream*)tldrZip;
    tldrZip1->write(buffer, bytes);
    return bytes;
}

string downloadZip() {
    CURL* curl = curl_easy_init();
    if(!curl) {
        return "";
    }
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, "https://tldr.sh/assets/tldr.zip");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, processChunk);
    std::ofstream tldrZip("/tmp/tldr.zip");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&tldrZip);
    res = curl_easy_perform(curl);
    tldrZip.close();
    if(res != CURLE_OK) {
        return "";
    }
    curl_easy_cleanup(curl);
    return "/tmp/tldr.zip";
}

void updateCache() {

    downloadZip();

    // remove any ~/.tldr/cache.old directory, just to make sure
    std::filesystem::remove_all(global::HOME + "/.tldr/cache.old");

    // rename cache to cache.old
    std::filesystem::rename(global::HOME + "/.tldr/cache", global::HOME + "/.tldr/cache.old");

    // create new empty cache directory
    std::filesystem::create_directory(global::tldrPath);

    int err = 0; // TODO
    zip* archive = zip_open("/tmp/tldr.zip", 0, &err);
	if(!archive) {
		throw std::runtime_error("Could not open archive");
	}
    struct zipEntry entry;

	zip_int64_t numEntries = zip_get_num_entries(archive, 0);
    std::cout << numEntries << std::endl;
	for(int i = 0; i < numEntries; i++) {
        entry.path = zip_get_name(archive, i, 0);
        parseEntry(entry);
        if(!entry.wanted) {
            continue;
        }
        if(entry.isFile) {
            zip_file_t* file = zip_fopen(archive, entry.path.c_str(), 0);
	        struct zip_stat	st;
		    zip_stat_init(&st);
		    zip_stat_index(archive, i, 0, &st);
		    char* content = new char[st.size];
		    zip_fread(file, content, st.size);
            std::ofstream outfile(global::tldrPath + entry.path);
            outfile.write(content, st.size);
            outfile.close();
		    zip_fclose(file);
		    delete[] content;
        } else {
            std::filesystem::create_directory(global::tldrPath + entry.path);
        }
	}
	zip_close(archive);
    std::filesystem::remove_all(global::HOME + "/.tldr/cache.old");
}