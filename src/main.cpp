#include <iostream>
#include <vector>
#include <filesystem>

#include "opts.h"
#include "cache.h"
#include "global.h"
#include "update.h"
#include "page.h"
#include "find/find.h"

#define trycatch(...)	try {__VA_ARGS__;} \
						catch (const std::runtime_error& e) { \
					  		std::cerr << e.what() << std::endl; \
							return 1; \
						}

void displayHelp();

int main(int argc, char *argv[]) {

	opts::parse(argc, argv);

	if(global::opts::help) {
		displayHelp();
		return 0;
	}

	trycatch(global::init());

	if(global::opts::find) {
		find(global::opts::search_terms);
		/*for(auto & s : global::opts::search_terms) {
			stem(s);
		}*/
		return 0;
	}

	if(global::opts::render) {
		Page page;
		trycatch(page = cache::getPageFromPath(global::opts::file));
		if(!global::opts::raw) {
			page.format();
		}
		page.print();
		return 0;
	}

	if(global::opts::update) {
		trycatch(updateCache())
		return 0;
	}

	trycatch(cache::verify());

	if(global::opts::stat) {
		trycatch(cache::stat(global::opts::file));
		return 0;
	}

	trycatch(cache::findPlatforms());

	string filePath;

	Page page;
	trycatch(page = Page(cache::getPage(global::opts::file)));

	if(!global::opts::raw) {
		page.format();
	}
	page.print();

	return 0;
}

void displayHelp() {
	std::cout <<
		"Usage: tldr [options] [command]\n"
		"\n"
		"Options:\n"
		"  -u, --update:   Update all currently installed languages in the local tldr cache\n"
		"  -l, --language: Specify a comma-separated list of language codes, like \"fr,it\"\n"
		"                  Pages will be searched for in this order. If nothing is found it will default to \"en\"\n"
		"                  When used with --update, this will specify the languages to download\n"
		"  -p, --platform  Override the default platform\n"
		"  -v, --verbose:  When used with --update, this will print every file, wich was created or modified\n"
		"                  When used with --platform, it will show a message when the page was not found in that platform\n"
		"  -s, --stat      Show the translation status of a command. Results can be narrowed down by --language and --platform\n"
		"  -r, --render    Specify the path to a custom Markdown page to render\n"
		"      --raw       Display the raw Markdown page without formatting\n"
		"  -a, --all       When used with --update, this updates all languages, instead of just the installed ones\n"
		"  -h, --help:     Display this help" << std::endl;
}