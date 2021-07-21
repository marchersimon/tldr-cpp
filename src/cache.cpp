#include "cache.h"

void cache::Structure::sortPlatforms() {
	for(auto & platform : platforms) {
		if(platform.name == global::defaultPlatform) {
			std::swap(platforms.at(0), platform);
		}
		if(platform.name == "common") {
			std::swap(platforms.at(1), platform);
		}
	}
}

cache::Structure cache::check() { // TODO: add support for OSX and Windows
	// get path to $HOME
	char* HOME = getenv("HOME");
	if(HOME == nullptr) {
		throw std::runtime_error("Environment variable $HOME not set");
	}

	// check if page cache exists at $HOME/.cache/tldr
	global::tldrPath = string(HOME) + "/.cache/tldr/pages";
	struct stat statStruct;
	if(stat(global::tldrPath.c_str(), &statStruct) != 0) {
		throw std::runtime_error("no tldr");
	}
	if(!(statStruct.st_mode & S_IFDIR)) {
		throw std::runtime_error(global::tldrPath + " is not a directory");
	}

	// get the structure of the tldr cache (platforms and number of pages per platform)
	cache::Structure tldrStructure;

	for(const auto & entry : std::filesystem::directory_iterator(global::tldrPath)) {// needs C++17
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

string cache::getPage(string name, std::vector<cache::Platform> platforms) {
	struct stat statStruct;
	string filePath;
	for(const auto & platform : platforms) {
		filePath = global::tldrPath + "/" + platform.name + "/" + name + ".md";
		if(stat(filePath.c_str(), &statStruct) == 0 && statStruct.st_mode & S_IFREG) {
			break;
		}
		filePath = "";
	}
	
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
	return fileContent;
}