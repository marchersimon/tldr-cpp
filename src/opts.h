#pragma once

#include <getopt.h>
#include <string>
#include <vector>

#include "global.h"

namespace opts {

	inline bool help = false;
	inline bool update = false;
	inline std::string file;
	inline std::vector<string> languages;
	inline bool verbose = false;
	inline string platform;
	inline bool raw = false;

	/*
	Parses a comma-separated list of languages like "en,fr,it" and puts them in a list
	*/
	inline void parseLanguages(char* optarg) {
		languages.clear();
		std::stringstream languagelist(optarg);
		while(languagelist.good()) {
			string nextLanguage;
			std::getline(languagelist, nextLanguage, ',');
			languages.push_back(nextLanguage);
		}
	}

	inline void parse(int argc, char *argv[]) {

		static struct option long_options[] = {
			{"help", no_argument, NULL, 'h'},
			{"update", no_argument, NULL, 'u'},
			{"language", required_argument, NULL, 'l'},
			{"verbose", no_argument, NULL, 'v'},
			{"platform", required_argument, NULL, 'p'},
			{"raw", no_argument, NULL, 'r'},
			{NULL, 0, NULL, 0},
		};

		char opt;

		while((opt = getopt_long(argc, argv, "hul:vp:", long_options, NULL)) != -1) {
			switch(opt) {
				case 'h':
					help = true;
					break;
				case 'u':
					update = true;
					break;
				case 'l':
					parseLanguages(optarg);
					break;
				case 'v':
					verbose = true;
					break;
				case 'p':
					platform = optarg;
					break;
				case 'r':
					raw = true;
					break;
			}
		}

		if(optind < argc) { // if there are remaining arguments -> the page name
			file = argv[optind];
		}
		for(int i = optind + 1; i < argc; i++) {
			file.append("-");
			file.append(argv[i]);
		}
	}
}
