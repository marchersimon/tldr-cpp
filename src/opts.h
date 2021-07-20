#pragma once

#include <getopt.h>

namespace opts {

	bool help = false;
	std::string file;

	void parse(int argc, char *argv[]) {

		static struct option long_options[] = {
			{"help", no_argument, NULL, 'h'},
			{NULL, 0, NULL, 0},
		};

		char opt;

		while((opt = getopt_long(argc, argv, "h", long_options, NULL)) != -1) {

			switch(opt) {
				case 'h':
					help = true;
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