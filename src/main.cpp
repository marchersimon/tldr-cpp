#include <iostream>
#include <vector>
#include <filesystem>

#include "opts.h"
#include "cache.h"
#include "global.h"
#include "update.h"
#include "page.h"


void displayHelp();

int main(int argc, char *argv[]) {

	opts::parse(argc, argv);

	if(opts::help) {
		displayHelp();
		return 0;
	}

	try {
		cache::init();
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	
	if(opts::stat) {
		try{
			cache::stat(opts::file);
		} catch (const std::runtime_error& e) {
			std::cerr << e.what() << std::endl;
			return 1;
		}
		return 0;
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

	string filePath;

	Page* page;
	try {
		page = new Page(getPage(opts::file, tldrStructure.platforms));
	} catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	if(!opts::raw) {
		page->format();
	}
	page->print();

	free(page);

	return 0;
}

void displayHelp() {
	std::cout <<
		"Usage: tldr [options] [command]\n"
		"\n"
		"Options:\n"
		"  -u, --update:   Update the local tldr cache\n"
		"  -l, --language: Specify a comma-separated list of language codes, like \"fr,it\"\n"
		"                  Pages will be searched for in this order. If nothing is found it will default to \"en\"\n"
		"                  When used with --update, this will specify the languages to download\n"
		"  -p, --platform  Override the default platform\n"
		"  -v, --verbose:  When used with --update, this will print every file, wich was created or modified\n"
		"                  When used with --platform, it will show a message when the page was not found in that platform\n"
		"  -s, --stat      Show the translation status of a command. Results can be narrowed down by --language and --platform\n"
		"      --raw       Display the raw Markdown page without formatting\n"
		"  -h, --help:     Display this help" << std::endl;
}