#include "cache.h"

/*
This function gets the path to the cache and makes sure it exists
*/
void cache::init() {
	// get path to $HOME
	global::HOME = getenv("HOME");
	if(global::HOME.empty()) {
		throw std::runtime_error("Environment variable $HOME not set");
	}

	// check if page cache exists at $HOME/.cache/tldr and create it if not
	global::tldrPath = global::HOME + "/.tldr/cache/";
	struct stat statStruct;
	if(stat(global::tldrPath.c_str(), &statStruct) != 0) {
		std::filesystem::create_directory(global::HOME + "/.tldr/");
		std::filesystem::create_directory(global::HOME + "/.tldr/cache/");
	}
}

/*
This sorts the list of platforms, so that the preferred one is at first position and "common" is at second
*/
void cache::Structure::sortPlatforms() {
	for(auto & platform : platforms) {
		if(platform.name == opts::platform) {
			std::swap(platforms.at(0), platform);
		}
		if(platform.name == "common") {
			std::swap(platforms.at(1), platform);
		}
	}
}

cache::Structure cache::check() { // TODO: add support for OSX and Windows

	// get the structure of the tldr cache (platforms and number of pages per platform)
	cache::Structure tldrStructure;

	for(const auto & entry : std::filesystem::directory_iterator(global::tldrPath + "pages/")) {// needs C++17
		cache::Platform newPlatform;
		string path = entry.path();
		int pos = path.find_last_of('/') + 1;
		newPlatform.name = path.substr(pos);
		for(const auto & subentry : std::filesystem::directory_iterator(path)) {
			newPlatform.numberOfPages++;
		}
		tldrStructure.platforms.push_back(newPlatform);
	}
	return tldrStructure;
}

Page cache::getPage(string name, std::vector<cache::Platform> platforms) {
	struct stat statStruct;
	string filePath;
	string platform;
	string pagesDir = "pages";
	std::vector<string> languages = opts::languages;
	if(languages.empty()) {
		languages.push_back("en");
	}
	for(const auto & language : languages) {
		if(language == "en") {
			pagesDir = "pages";
		} else {
			pagesDir = "pages." + language;
		}
		for(const auto & p : platforms) {
			filePath = global::tldrPath + pagesDir + "/" + p.name + "/" + name + ".md";
			if(stat(filePath.c_str(), &statStruct) == 0 && statStruct.st_mode & S_IFREG) {
				platform = p.name;
				goto page_found;
			}
			filePath = "";
		}
	}
	page_found:
	
	if(filePath.empty()) {
		throw std::runtime_error("Documentation for " + name + " is not available. ");
	}

	std::ifstream file(filePath);

	if(file.is_open() == false) {
		throw std::runtime_error("Could not open file " + filePath);
	}
	string line;
	string fileContent;
	while(std::getline(file, line)) {
		fileContent += line + '\n';
	}

	file.close();
	Page page(fileContent);
	page.platform = platform;
	return page;
}