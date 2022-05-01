#include "update.h"

/*
Parses the path to an entry in the zip file. It extracts the language code, platform and file name. 
The path can be "pages(.xyz)/", "pages(.xyz)/platform/" or "pages(.xyz)/platform/filename.md".
*/
void parseEntry(struct zipEntry &entry, vector<string> & installedLanguages) {

    entry.isFile = false;
    entry.wanted = false;
    entry.language = "";
    entry.platform = "";
    entry.filename = "";

    std::size_t start = 0;
    std::size_t end = 0;
    int i = 0;

    if(entry.path == "index.json" || entry.path == "LICENSE.md") {
        entry.wanted = true;
        entry.isFile = true;
        return;
    }
    
    // this should probably be rewritten sometime
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
    } while(end != string::npos);

    // only extract the page if the language was provided with -l or if nothing was provided check if the language was already installed
    if((entry.language == "en") || (global::opts::update_all == true) | (installedLanguages.empty())) {
        entry.wanted = true;
    } else if(global::opts::languages.empty()) {
        if(std::find(installedLanguages.begin(), installedLanguages.end(), entry.language) != installedLanguages.end()) {
            entry.wanted = true;
        }
    } else if(std::find(global::opts::languages.begin(), global::opts::languages.end(), entry.language) != global::opts::languages.end()){
        entry.wanted = true;
    }
}

int libcurl_ProgressCallback(void* ptr, double dltotal, double dlnow, double utotal, double ulnow) {
    // prevent division by zero later on in case the file is empty
    if(dltotal <= 0) {
        return 0;
    }

    int percentage = 100 * dlnow / dltotal;
    std::cout << "\r[1/2] Downloading: " << percentage << "%" << std::flush;

    return 0;
}

/*
This function gets called every time a new chunk is downloaded and adds it to the buffer
*/
size_t libcurl_WriteCallback(char* downloadBuffer, size_t itemsize, size_t nitems, void* bufferPtr) {
    size_t bytes = itemsize * nitems;
    vector<char>* buffer = (vector<char>*)bufferPtr;
    int offset = buffer->size();
    buffer->resize(offset + bytes);
    for(uint i = 0; i < bytes; i++) {
        buffer->at(i + offset) = downloadBuffer[i];
    }
    return bytes; // this is needed by libcurl
}

// This function downloads the zip file into memory
vector<char> downloadZip() {
    CURL* curl = curl_easy_init();
    if(!curl) {
        throw std::runtime_error("To put it in the words of the libcurl docs: Something went wrong");
    }

    vector<char> buffer;

    curl_easy_setopt(curl, CURLOPT_URL, "https://tldr.sh/assets/tldr.zip");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, libcurl_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&buffer);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, libcurl_ProgressCallback);
    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    
    switch(res) {
        case CURLE_OK:
            break;
        case CURLE_COULDNT_RESOLVE_HOST:
            throw std::runtime_error("Could not resolve host. Are you connected to the internet?");
            break;
        default:
            throw std::runtime_error("Could not download cache: Error code " + std::to_string(res) + ". More at https://curl.se/libcurl/c/libcurl-errors.html.");

    }
    return buffer;
}

void printDiff() {
    int tldrPathLenght = global::tldrPath.length();
    std::hash<std::string> hash;
    std::vector<string> platforms = {"common", "linux", "osx", "android", "windows", "sunos"};
    std::vector<string> languages = getInstalledLanguages();
    std::vector<pageUpdateStatus> changedPages;
    for(const auto & entry : std::filesystem::recursive_directory_iterator(global::tldrPath)) {// needs C++17
        if(entry.is_directory() || entry.path().filename() == "index.json") {
            continue;
        }
        std::ifstream fileStream1(entry.path());
        string file1;
        string temp;
        while(std::getline(fileStream1, temp)) {
            file1 += temp + '\n';
        }
        fileStream1.close();
        size_t hash1 = hash(file1);

        string oldPath = entry.path();
        oldPath.insert(tldrPathLenght - 1, ".old");
        std::ifstream fileStream2(oldPath);
        if(!fileStream2.is_open()) {
            if(global::opts::verbose) {
                string line = entry.path().string().substr(tldrPathLenght, entry.path().string().length() - tldrPathLenght);
                std::string language, platform, name;
                if(line.at(5) == '/') {
                    language = "en";
                } else {
                    language = line.substr(6,line.find_first_of('/')-6);
                }
                size_t pos = line.find('/');
                size_t pos1 = line.find('/', pos+1);
                platform = line.substr(pos+1, pos1 - pos - 1);
                name = line.substr(pos1+1, line.find('.', 6)-pos1-1);
                changedPages.push_back({name, language, platform, true});  
            }
            continue;
        }

        string file2;
        while(std::getline(fileStream2, temp)) {
            file2 += temp + '\n';
        }
        fileStream2.close();
        size_t hash2 = hash(file2);
        if(hash1 != hash2) {
            if(global::opts::verbose) {
                string line = entry.path().string().substr(tldrPathLenght, entry.path().string().length() - tldrPathLenght);
                std::string language, platform, name;
                if(line.at(5) == '/') {
                    language = "en";
                } else {
                    language = line.substr(6,line.find_first_of('/')-6);
                }
                size_t pos = line.find('/');
                size_t pos1 = line.find('/', pos+1);
                platform = line.substr(pos+1, pos1 - pos - 1);
                name = line.substr(pos1+1, line.find('.', 6)-pos1-1);
                changedPages.push_back({name, language, platform, false});
            }
        }
    }

    // print all changes
    bool languageExists;
    bool platformExists;
    for(const std::string & l : languages) {
        languageExists = false;
        for(const std::string & pl : platforms) {
            platformExists = false;
            for(const pageUpdateStatus & p : changedPages) {
                if(p.language == l && p.platform == pl) {
                    if(!languageExists) {
                        std::cout << l << "\n";
                        languageExists = true;
                    }
                    if(!platformExists) {
                        std::cout << "  " << pl << "\n";
                        platformExists = true;
                    }
                    std::cout << "    ";
                    if(p.created) {
                        std::cout << global::color::pageCreated;
                    } else {
                        std::cout << global::color::pageModified;
                    }
                    std::cout << p.name << global::color::dfault << "\n";
                }
            }
        }
    }
    std::cout << std::endl;

    if(changedPages.size() == 0) {
        // this will also be displayed if pages have been removed // TODO
        std::cout << "It had no effect!" << std::endl;
    } else if (changedPages.size() == 1) {
        std::cout << "1 page updated" << std::endl;
    } else {
        std::cout << changedPages.size() << " pages updated" << std::endl;
    }
}

vector<string> getInstalledLanguages() {
    vector<string> installedLanguages;
    for(const auto & entry : std::filesystem::directory_iterator(global::HOME + "/.tldr/cache.old/")) {// needs C++17
		string path = entry.path();
        std::size_t pos = path.find_last_of('/');
        if(path[pos + 1] != 'p') {// if it's not "pages"
            continue;
        }
        pos += 7; // skip "pages."
        if(pos == path.length() + 1) {
            installedLanguages.push_back("en");
            continue;
        }
        string language = path.substr(pos, path.length() - pos);
        installedLanguages.push_back(language);
	}
    return installedLanguages;
}

void updateCache() {

    vector<char> zipVector = downloadZip();
    std::cout << std::endl << "[2/2] Extracting..." << std::endl;

    // remove any ~/.tldr/cache.old directory, just to make sure
    std::filesystem::remove_all(global::HOME + "/.tldr/cache.old");

    // rename cache to cache.old
    std::filesystem::rename(global::HOME + "/.tldr/cache", global::HOME + "/.tldr/cache.old");

    // create new empty cache directory
    std::filesystem::create_directory(global::tldrPath);

    char* zipBuffer = &zipVector[0];
    zip_error_t err; // TODO
    zip_source_t* zipsource = zip_source_buffer_create((void*)zipBuffer, zipVector.size(), 0, &err);
    zip* archive = zip_open_from_source(zipsource, 0, &err);
	if(!archive) {
		throw std::runtime_error("Error opening the downloaded archive");
	}
    struct zipEntry entry;

    vector<string> installedLanguages = getInstalledLanguages(); // needed in parseEntry()

	zip_int64_t numEntries = zip_get_num_entries(archive, 0);
	for(int i = 0; i < numEntries; i++) {
        entry.path = zip_get_name(archive, i, 0);
        parseEntry(entry, installedLanguages);
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
    zipVector.clear();
    printDiff();
    std::filesystem::remove_all(global::HOME + "/.tldr/cache.old");
}
