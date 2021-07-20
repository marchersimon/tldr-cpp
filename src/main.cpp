#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <fstream>
#include <filesystem>

#include "opts.h"

using std::string;

namespace global {
	string tldrPath;
	string defaultPlatform = "linux";
}

class Platform {
	public:
		string name;
		int numberOfPages = 0;
};

class TldrStructure {
	public:
		std::vector<Platform> platforms;
		void sortPlatforms() {
			for(auto & platform : platforms) {
				if(platform.name == global::defaultPlatform) {
					std::swap(platforms.at(0), platform);
				}
				if(platform.name == "common") {
					std::swap(platforms.at(1), platform);
				}
			}
		}
};

TldrStructure checkTldrCache() { // TODO: add support for OSX and Windows
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
	TldrStructure tldrStructure;

	for(const auto & entry : std::filesystem::directory_iterator(global::tldrPath)) {// needs C++17
		Platform newPlatform;
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

string getPage(string name, std::vector<Platform> platforms) {
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

void displayHelp() {
	std::cout <<
		"Usage: tldr [options] command\n"
		"\n"
		"Options:\n"
		"  -h, --help: Display this help\n";
}

int main(int argc, char *argv[]) {

	opts::parse(argc, argv);

	if(opts::file.empty() || opts::help) {
		displayHelp();
		return 0;
	}

	TldrStructure tldrStructure;

	try{
		tldrStructure = checkTldrCache();
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	tldrStructure.sortPlatforms();

	string filePath;

	try {
		filePath = getPage(opts::file, tldrStructure.platforms);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	std::cout << filePath << std::endl;

	return 0;
}