#pragma once

#include <getopt.h>

namespace opts {

	inline bool help = false;
	inline bool update = false;
	inline std::string file;
	inline bool overrideLanguage = false;
	inline std::string language;
	inline bool verbose;

	inline void parse(int argc, char *argv[]) {

		static struct option long_options[] = {
			{"help", no_argument, NULL, 'h'},
			{"update", no_argument, NULL, 'u'},
			{"language", required_argument, NULL, 'u'},
			{"verbose", no_argument, NULL, 'v'},
			{NULL, 0, NULL, 0},
		};

		char opt;

		while((opt = getopt_long(argc, argv, "hul:v", long_options, NULL)) != -1) {
			switch(opt) {
				case 'h':
					help = true;
					break;
				case 'u':
					update = true;
					break;
				case 'l':
					overrideLanguage = true;
					language = optarg;
					break;
				case 'v':
					verbose = true;
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
