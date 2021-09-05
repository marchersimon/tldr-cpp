#include "pr.h"

size_t libcurl_WriteCallback_pr(char* downloadBuffer, size_t itemsize, size_t nitems, void* bufferPtr) {
    size_t bytes = itemsize * nitems;
    string* buffer = (string*)bufferPtr;
    int offset = buffer->length();
    buffer->resize(offset + bytes);
    for(uint i = 0; i < bytes; i++) {
        buffer->at(i + offset) = downloadBuffer[i];
    }
    return bytes; // this is needed by libcurl
}

string download(string url) {
    CURL* curl = curl_easy_init();
    if(!curl) {
        throw std::runtime_error("To put it in the words of the libcurl docs: Something went wrong");
    }

    string buffer;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, libcurl_WriteCallback_pr);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "request");
    CURLcode res = curl_easy_perform(curl);

    switch(res) {
        case CURLE_OK:
            break;
        case CURLE_COULDNT_RESOLVE_HOST:
            throw std::runtime_error("Could not resolve host. Are you connected to the internet?");
            break;
        default:
            throw std::runtime_error("Could not download cache: Error code " + std::to_string(res) +
                ". More at https://curl.se/libcurl/c/libcurl-errors.html.");
    }
    return buffer;
}

void printPR() {
  
    string buffer = download("https://api.github.com/repos/tldr-pages/tldr/pulls/" + global::opts::prNumber);

    if(buffer.starts_with("{\"message\":\"Not Found\"")) {
        string msg = "#" + global::opts::prNumber + " is not a Pull Request";
        throw std::runtime_error(msg);
    }

    size_t pos = buffer.find("\"state\":");
    if(buffer.substr(pos + 9, 6) == "closed") {
        throw std::runtime_error("Pull request #" + global::opts::prNumber + " is already closed");
    }

    string diffURL = "https://patch-diff.githubusercontent.com/raw/tldr-pages/tldr/pull/" + global::opts::prNumber + ".diff";
    string diffBuffer = download(diffURL);

    vector<string> changedPages;
    pos = diffBuffer.find("diff --git a/pages");
    while(pos != string::npos) {
        pos += 13;
        size_t endPos = diffBuffer.find(" ", pos);
        string changedPage = diffBuffer.substr(pos, endPos - pos);
        changedPages.push_back(changedPage);
        pos = diffBuffer.find("diff --git a/pages", endPos);
    }

    if(changedPages.size() == 0) {
        throw std::runtime_error("This PR doesn't change any tldr pages");
    }

    pos = buffer.find("\"head\":{");
    pos = buffer.find("\"ref\":", pos);
    pos += 7;
    size_t endPos = buffer.find("\"", pos);
    string branch = buffer.substr(pos, endPos - pos);
    pos = buffer.find("\"repo\":", endPos);
    pos = buffer.find("\"full_name\":", pos);
    pos += 13;
    endPos = buffer.find("\"", pos);
    string slug = buffer.substr(pos, endPos - pos);

    if(global::opts::file.empty()) {
        if(changedPages.size() == 1) {
            string url = "https://raw.githubusercontent.com/" + slug + "/" + branch + "/" + changedPages[0];
            Page page(download(url));
            page.format();
            page.print();
            return;
        } else {
            std::cout << "This PR modifies multiple tldr pages:" << std::endl;
            for(const string & page : changedPages) {
                std::cout << page << std::endl;
            }
            std::cout << "Please specify one by running " << global::color::command << "tldr --pr "
                << global::opts::prNumber << " pages.xy/platform/page.md" << std::endl;
            return;
        }
    } else {
        // check if page was modified in PR
        bool modified = false;
        for(const string & page : changedPages) {
            if(global::opts::file == page) {
                modified = true;
                break;
            }
        }
        if(!modified) {
            throw std::runtime_error(global::opts::file + " wasn't modified in #" + global::opts::prNumber);
        }
        string url = "https://raw.githubusercontent.com/" + slug + "/" + branch + "/" + global::opts::file;
        Page page(download(url));
        page.format();
        page.print();
        return;
    }


}