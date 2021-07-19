#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <filesystem>

class Platform {
	public:
		std::string name;
		int numberOfPages = 0;
};

class TldrStructure {
	public:
		std::vector<Platform> platforms;
};

TldrStructure checkTldrCache() { // TODO: add support for OSX and Windows
	// get path to $HOME
	char* HOME = getenv("HOME");
	if(HOME == nullptr) {
		throw std::runtime_error("Environment variable $HOME not set");
	}

	// check if page cache exists at $HOME/.cache/tldr
	std::string tldrPath = std::string(HOME) + "/.cache/tldr/pages";
	struct stat fileStruct;
	if(stat(tldrPath.c_str(), &fileStruct) != 0) {
		throw std::runtime_error("no tldr");
	}
	if(!(fileStruct.st_mode & S_IFDIR)) {
		throw std::runtime_error(tldrPath + " is not a directory");
	}

	// get the structure of the tldr cache (platforms and number of pages per platform)
	TldrStructure tldrStructure;

	for(const auto & entry : std::filesystem::directory_iterator(tldrPath)) {// needs C++17
		Platform newPlatform;
		std::string path = entry.path();
		int pos = path.find_last_of('/') + 1;
		newPlatform.name = path.substr(pos);
		for(const auto & subentry : std::filesystem::directory_iterator(path)) {
			newPlatform.numberOfPages++;
		}
		tldrStructure.platforms.push_back(newPlatform);
	}
	return tldrStructure;
}

int main() {

	TldrStructure tldrStructure;

	try{
		tldrStructure = checkTldrCache();
	} catch (const std::runtime_error& e) {
		std::cout << e.what() << std::endl;
		return 1;
	}

	int size = tldrStructure.platforms.size();
	for(int i = 0; i < size; i++) {
		std::cout << tldrStructure.platforms.at(i).name << " " << tldrStructure.platforms.at(i).numberOfPages << std::endl;
	}

	return 0;
}