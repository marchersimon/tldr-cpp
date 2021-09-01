#include "cache.h"

/*
This function makes sure the local cache isn't empty
*/
void cache::verify() {
	struct stat statStruct;
	string tldrPagePath = global::tldrPath + "pages/";
	if(stat(tldrPagePath.c_str(), &statStruct) != 0) {
		throw std::runtime_error("The local cache is still empty. Run tldr -u to update it");
	}
}

/*
This function returns a sorted list of installed platforms
*/
void cache::findPlatforms() {

	vector<string> platforms;

	// get list of platforms
	for(const auto & entry : std::filesystem::directory_iterator(global::tldrPath + "pages/")) {// needs C++17
		string newPlatform;
		string path = entry.path();
		int pos = path.find_last_of('/') + 1;
		newPlatform = path.substr(pos);
		platforms.push_back(newPlatform);
	}

	// sort the list, so that the preferred one will be first and common will be second
	for(auto & platform : platforms) {
		if(platform == global::opts::platform) {
			std::swap(platforms.at(0), platform);
		}
		if(platform == "common") {
			std::swap(platforms.at(1), platform);
		}
	}

	global::platforms = platforms;
}

cache::Index::Target::Target(string platform, string language) {
	this->language = language;
	this->platform = platform;
}

/*
This function checks wheter a page in the index exists in a specific language-platform combination
*/
bool cache::Index::contains(cache::Index::Target target) {
	for(auto & t : targets) {
		if(t.language == target.language && t.platform == target.platform) {
			return true;
		}
	}
	return false;
}

/*
This function searches index.json for a specific page and returns all its targets as Index object
*/
cache::Index cache::getFromIndex(string name) {
	std::ifstream file(global::tldrPath + "index.json");
	if(!file.is_open()) {
		throw std::runtime_error("Could not open " + global::tldrPath + "index.json");
	}
	string fileContent;
	std::getline(file, fileContent); // whole file only contains 1 newline
	file.close();
	// find the page in the file
	uint pos = fileContent.find("\"" + name + "\"");
	if(pos == string::npos) {
		throw std::runtime_error("404");
	}
	pos = fileContent.find("\"targets\"", pos);
	int nextPos = fileContent.find("]", pos);
	// extract that part with all the targets
	string targets = fileContent.substr(pos + 11, nextPos - (pos + 11));
	fileContent.clear(); // to save memory

	pos = 7;
	cache::Index index;
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

/*
This function gets a Page from any path. Used for --render
*/
Page cache::getPageFromPath(string path) {
	std::ifstream file(path);
	if(!file.is_open()) {
		throw std::runtime_error("File " + path + " could not be opened");
	}

	string line;
	string fileContent;
	while(std::getline(file, line)) {
		fileContent += line + '\n';
	}
	file.close();
	Page page(fileContent);
	return page;
}

/*
This function gets the content of a specific page in the correct language from the correct directory and returns it as Page object
*/
Page cache::getPage(string name) {
	struct stat statStruct;
	string filePath;
	string platform;
	string language;
	std::vector<string> languages = global::opts::languages;
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

	for(const auto & p : global::platforms) {
		for(const auto & l : languages) {
			if(index.contains({p, l})) {
				if(l == "en") {
					filePath = global::tldrPath + "pages/" + p + "/" + name + ".md";
				} else {
					filePath = global::tldrPath + "pages." + l + "/" + p + "/" + name + ".md";
				}
				platform = p;
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

/*
Print platform and translation status of a specific page
*/
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
									 "pt_BR", "pt_PT", "ro", "ru", "sh", "sv", "ta", "th", "tr", "zh", "zh_TW"};
	if(!global::opts::languages.empty()) {
		languages = global::opts::languages;
	}
	std::vector<string> platforms = {"common", "linux", "osx", "android", "windows", "sunos"};
	if(!global::opts::platform.empty()) {
		platforms.clear();
		platforms.push_back(global::opts::platform);
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
				std::cout << global::color::stat_checkmark + "\xE2\x9C\x94" + global::color::dfault; // check mark
				for(uint i = 0; i < l.length(); i++) {
					std::cout << " ";
				}
			} else {
				for(uint i = 0; i < l.length() + 1; i++) {
					std::cout << " ";
				}
			}
		}
		std::cout << std::endl;
	}
}