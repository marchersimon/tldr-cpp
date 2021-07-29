#include <iostream>
#include <vector>
#include <filesystem>

#include "opts.h"
#include "cache.h"
#include "global.h"
#include "update.h"
#include "page.h"


void displayHelp() {
	std::cout <<
		"Usage: tldr [options] command\n"
		"\n"
		"Options:\n"
		"  -h, --help:     Display this help\n"
		"  -u, --update:   Update the local tldr cache\n"
		"  -l, --language: Override the default language for pages or, when used with --update, override the default language (if any) for downloading pages.\n"
		"                  If the specified language isn't found, it will be ignored.\n"
		"  -v, --verbose:  Print more details\n";
}

void init() {
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

int main(int argc, char *argv[]) {

	opts::parse(argc, argv);

	if(opts::help) {
		displayHelp();
		return 0;
	}
	
	try {
		init();
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	if(opts::update) {
		try{
			updateCache();
		} catch (const std::runtime_error& e) {
			std::cerr << e.what() << std::endl;
			return 1;
		}
		return 0;
	}

	cache::Structure tldrStructure;

	try{
		tldrStructure = cache::check();
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	tldrStructure.sortPlatforms();

	global::defaultPlatform = "linux";

	string filePath;

	try {
		filePath = getPage(opts::file, tldrStructure.platforms);
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	Page page(filePath);
	page.format();
	page.print();

	return 0;
}