#include <iostream>
#include <vector>

#include "opts.h"
#include "cache.h"
#include "global.h"

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

	std::cout << filePath << std::endl;

	return 0;
}