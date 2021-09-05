#include "global.h"

#include <sys/stat.h>
#include <iostream>
#include <filesystem>

namespace global {

    vector<string> platforms;

	namespace opts {
		bool help = false;
		bool update = false;
		string file;
		vector<string> languages;
		bool verbose = false;
		string platform;
		bool raw = false;
		bool stat = false;
		bool render = false;
		bool update_all = false;
		bool find = false;
		vector<string> search_terms;
		bool findOverrideDefaults = false;
		bool name = false; 
		bool description = false;
		bool examples = false;
		bool stem = false;
		bool sub = false;
		bool destroy = false;
		bool pr = false;
		string prNumber;
	}

	string tldrPath;
	string HOME;
	namespace color {
		string dfault = "\033[0m";
		string title = "\033[1;38;2;200;50;50m"; // bold;set-fg-color;rgb;<r>;<g>;<b>m
		string backtick = "\033[38;5;110m";
		string command = "\033[38;2;200;100;150m";
		string token = "\033[38;2;40;215;66m";
		string stat_checkmark = "\033[1;32m";
		string foundMatch = "\033[38;5;110m";
	}
}

/*
This function gets the full path to the cache and makes sure it exists
*/
void global::init() {
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