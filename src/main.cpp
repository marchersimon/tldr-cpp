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

	if(global::opts::sub) {
		cache::printSubcommands(global::opts::file);
		return 0;
	}

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

	if(global::opts::stem) {
		if(!global::opts::languages.empty()) {
			std::cout << "Option --stem is only available for English pages!" << std::endl;
			return 0;
		}
		printStemmedPage(page);
		return 0;
	}

	if(!global::opts::raw) {
		page.format();
	}
	page.print();

	return 0;
}

void displayHelp() {
	string generalHelp = 
		"# tldr\n"
		"\n"
		"> View tldr pages in the command-line.\n"
		"> More information: <https://github.com/marchersimon/tldr-cpp>.\n"
		"\n"
		"- Show the tldr page of a specific command:\n"
		"\n"
		"`tldr {{command}}`\n"
		"\n"
		"- Update the local tldr cache:\n"
		"\n"
		"`tldr --update`\n"
		"\n"
		"- Show a tldr page, overriding the default platform:\n"
		"\n"
		"`tldr --platform {{android|linux|osx|sunos|windows}} {{command}}`\n"
		"\n"
		"- Show a tldr page, overriding the preferred languge and falling back to English in case none of them are found:\n"
		"\n"
		"`tldr --language {{de,fr,it}}`\n"
		"\n"
		"- Show the translation progress of a specific tldr page:\n"
		"\n"
		"`tldr --stat {{command}}`\n"
		"\n"
		"- Render a Markdown tldr page at a specific location:\n"
		"\n"
		"`tldr --render {{path/to/tldr_page.md}}`\n"
		"\n"
		"- Print the raw Markdown source of a tldr page:\n"
		"\n"
		"`tldr --raw {{command}}`\n"
		"\n"
		"- Show help for a specific topic:\n"
		"\n"
		"`tldr --help --update|--find`\n";
		
	string updateHelp =
		"# tldr --update\n"
		"\n"
		"> Update the local tldr cache.\n"
		"> More information: <https://github.com/marchersimon/tldr-cpp>.\n"
		"\n"
		"- Update all currently installed languages:\n"
		"\n"
		"`tldr --update`\n"
		"\n"
		"- Update only specific languages and remove the others (`en` will always be installed):\n"
		"\n"
		"`tldr --update --language {{zh,es,pl}}`\n"
		"\n"
		"- Update all languages, not only installed ones:\n"
		"\n"
		"`tldr --update --all`\n"
		"\n"
		"- Update all currently installed pages, showing each created or modified page:\n"
		"\n"
		"`tldr --update --verbose`\n";

	string findHelp =
		"# tldr --find\n"
		"\n"
		"> Perform a full-text search on all installed English tldr pages.\n"
		"> More information: <https://github.com/marchersimon/tldr-cpp>.\n"
		"\n"
		"- Search for a query in the whole page:\n"
		"\n"
		"`tldr --find {{show current time}}`\n"
		"\n"
		"- Search for a query in all page names and/or the command description and/or all command descriptions:\n"
		"\n"
		"`tldr --name|--description|--examples --find {{git commit}}`\n"
		"\n"
		"- Search for a query in pages from a specific platform:\n"
		"\n"
		"`tldr --platform {{linux}} --find {{shut down}}`\n"
		"\n"
		"- Serch for a query and print the weighing of each term and the score of the matches:\n"
		"\n"
		"`tldr --verbose --find {{query}}`\n";
		
		
		if(global::opts::update) {
			Page page(updateHelp);
			page.format();
			page.print();
			return;
		} else if(global::opts::find) {
			Page page(findHelp);
			page.format();
			page.print();
		} else {
			Page page(generalHelp);
			page.format();
			page.print();
		}

		/*
		"Usage: tldr [options] [command]\n"
		"\n"
		"Options:\n"
		"  -u, --update:   Update all currently installed languages in the local tldr cache\n"
		"  -l, --language: Specify a comma-separated list of language codes, like \"fr,it\"\n"
		"                  Pages will be searched for in this order. If nothing is found it will default to \"en\"\n"
		"                  When used with --update, this will specify the languages to download\n"
		"  -p, --platform  Override the default platform\n"
		"  -v, --verbose  When used with --update, this will print every file, wich was created or modified\n"
		"                  When used with --platform, it will show a message when the page was not found in that platform\n"
		"  -s, --stat      Show the translation status of a command. Results can be narrowed down by --language and --platform\n"
		"  -r, --render    Specify the path to a custom Markdown page to render\n"
		"      --raw       Display the raw Markdown page without formatting\n"
		"  -a, --all       When used with --update, this updates all languages, instead of just the installed ones\n"
		"  -h, --help:     Display this help";
		*/
}