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

cache::Index::Target::Target(string platform, string language) {
	this->language = language;
	this->platform = platform;
}

bool cache::Index::contains(cache::Index::Target target) {
	for(auto & t : targets) {
		if(t.language == target.language && t.platform == target.platform) {
			return true;
		}
	}
	return false;
}

cache::Index cache::getFromIndex(string name) {
	cache::Index index;
	std::ifstream file(global::tldrPath + "index.json");
	if(!file.is_open()) {
		throw std::runtime_error("Could not open " + global::tldrPath + "index.json");
	}
	string fileContent;
	std::getline(file, fileContent); // whole file only contains 1 newline
	file.close();
	int pos = fileContent.find("\"" + name + "\"");
	if(pos == string::npos) {
		throw std::runtime_error("404");
	}
	pos = fileContent.find("\"targets\"", pos);
	int nextPos = fileContent.find("]", pos);
	string targets = fileContent.substr(pos + 11, nextPos - (pos + 11));
	fileContent.clear(); // to save memory

	pos = 7;
	while(pos < targets.size()) {
		nextPos = targets.find("\"", pos);
		string platform = targets.substr(pos, nextPos - pos);
		pos = nextPos + 14;
		nextPos = targets.find("\"", pos);
		string language = targets.substr(pos, nextPos - pos);
		index.targets.push_back({platform, language}); // automatically calls Target(platform, language)
		pos = nextPos + 10;
	}

	return index;
}

Page cache::getPage(string name, std::vector<cache::Platform> platforms) {
	struct stat statStruct;
	string filePath;
	string platform;
	string language;
	std::vector<string> languages = opts::languages;
	languages.push_back("en");

	cache::Index index;
	try {
		index = getFromIndex(name);
	} catch (const std::runtime_error& e) {
		if(string(e.what()) == "404") {
			throw std::runtime_error("Documentation for " + name + " is not available");
		} else {
			throw e;
		}
	}

	for(const auto & p : platforms) {
		for(const auto & l : languages) {
			if(index.contains({p.name, l})) {
				if(l == "en") {
					filePath = global::tldrPath + "pages/" + p.name + "/" + name + ".md";
				} else {
					filePath = global::tldrPath + "pages." + l + "/" + p.name + "/" + name + ".md";
				}
				platform = p.name;
				language = l;
				goto foundFile;
			}
		}
	}

	std::cout << "You shouldn't be able to see this." << std::endl;

	foundFile:

	std::ifstream file(filePath);

	if(file.is_open() == false) {
		if(language == "en") {
			throw std::runtime_error(filePath + " dissapeared. Try updating your cache.");
		} else {
			string languageDir = global::tldrPath + "pages." + language;
			if(stat(languageDir.c_str(), &statStruct) != 0) {
				throw std::runtime_error("Language " + language + " is not installed. You can install it with \"tldr -l " + language + " -u\" or install all languages with \"tldr -u\"");
			} else {
				throw std::runtime_error(filePath + " dissapeared. Try updating your cache.");
			}
		}
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

void cache::stat(string name) {
	cache::Index index;
	try {
		index = getFromIndex(name);
	} catch (const std::runtime_error& e) {
		if(string(e.what()) == "404") {
			throw std::runtime_error(name + " does not exist (yet)");
		} else {
			throw e;
		}
	}

	std::vector<string> languages = {"en", "bs", "da", "de", "es", "fa", "fr", "hi", "id", "it", "ja", "ko", "ml", "nl", "no", "pl",
									 "pt_BR", "pt_PT", "ru", "sh", "sv", "ta", "th", "tr", "zh", "zh_TW"};
	if(!opts::languages.empty()) {
		languages = opts::languages;
	}
	std::vector<string> platforms = {"common", "linux", "osx", "android", "windows", "sunos"};
	if(!opts::platform.empty()) {
		platforms.clear();
		platforms.push_back(opts::platform);
	}

	std::cout << std::endl << "         ";

	for(const auto & l : languages) {
		std::cout << l << " ";
	}

	std::cout << std::endl;
	for(const auto & p : platforms) {
		std::cout << p;
		for(int i = p.length(); i < 9; i++) {
			std::cout << " ";
		}
		for(const auto & l : languages) {
			if(index.contains({p, l})) {
				std::cout << "\xE2\x9C\x93"; // check mark
				for(int i = 0; i < l.length(); i++) {
					std::cout << " ";
				}
			} else {
				for(int i = 0; i < l.length() + 1; i++) {
					std::cout << " ";
				}
			}
		}
		std::cout << std::endl;
	}
}