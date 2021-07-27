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
    if(opts::overrideLanguage) {
        if(entry.language == opts::language || entry.language == "en") {
            entry.wanted = true;
        }
    } else {
        entry.wanted = true;
    }
}

/*
This function gets called every time a new chunk is downloaded and writes it to the filestream tldrZip
*/
size_t processChunk(char* buffer, size_t itemsize, size_t nitems, void* tldrZipPtr) {
    size_t bytes = itemsize * nitems;
    std::ofstream* tldrZip = (std::ofstream*)tldrZipPtr;
    tldrZip->write(buffer, bytes);
    return bytes; // this is needed by libcurl
}

// This function downloads the zip file to a specific path
void downloadZip(string path) {
    CURL* curl = curl_easy_init();
    if(!curl) {
        throw std::runtime_error("To put it in the words of the libcurl docs: Something went wrong");
    }
    std::ofstream tldrZip(path);

    curl_easy_setopt(curl, CURLOPT_URL, "https://tldr.sh/assets/tldr.zip");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, processChunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&tldrZip);
    CURLcode res = curl_easy_perform(curl);

    tldrZip.close();
    curl_easy_cleanup(curl);
    if(res != CURLE_OK) {
        throw std::runtime_error("Could not download cache: Error code " + std::to_string(res) + ". More at https://curl.se/libcurl/c/libcurl-errors.html.");
    }
}

void updateCache() {

    string zipPath = "/tmp/tldr.zip";
    downloadZip(zipPath);

    // remove any ~/.tldr/cache.old directory, just to make sure
    std::filesystem::remove_all(global::HOME + "/.tldr/cache.old");

    // rename cache to cache.old
    std::filesystem::rename(global::HOME + "/.tldr/cache", global::HOME + "/.tldr/cache.old");

    // create new empty cache directory
    std::filesystem::create_directory(global::tldrPath);

    int err = 0; // TODO
    zip* archive = zip_open("/tmp/tldr.zip", 0, &err);
	if(!archive) {
		throw std::runtime_error("Could not open archive at " + zipPath);
	}
    struct zipEntry entry;

	zip_int64_t numEntries = zip_get_num_entries(archive, 0);
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
    std::filesystem::remove(zipPath);
    std::filesystem::remove_all(global::HOME + "/.tldr/cache.old");
}