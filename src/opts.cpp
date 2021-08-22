#include "opts.h"

using namespace global::opts;

/*
Parses a comma-separated list of languages like "en,fr,it" and puts them in a list
*/
void opts::parseLanguages(char* optarg) {
	languages.clear();
	std::stringstream languagelist(optarg);
	while(languagelist.good()) {
		string nextLanguage;
		std::getline(languagelist, nextLanguage, ',');
		languages.push_back(nextLanguage);
	}
}

/*
Parses all command line argument and saves them to global::opts::...
*/
void opts::parse(int argc, char* argv[]) {

	static struct option long_options[] = {
    	{"help", no_argument, NULL, 'h'},
	    {"update", no_argument, NULL, 'u'},
	    {"language", required_argument, NULL, 'l'},
		{"verbose", no_argument, NULL, 'v'},
		{"platform", required_argument, NULL, 'p'},
		{"raw", no_argument, NULL, 'm'},
		{"stat", required_argument, NULL, 's'},
		{"render", required_argument, NULL, 'r'},
		{"all", no_argument, NULL, 'a'},
		{NULL, 0, NULL, 0},
	};

	char opt;

	while((opt = getopt_long(argc, argv, "hul:vp:s:r:a", long_options, NULL)) != -1) {
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
			case 'm':
				raw = true;
				break;
			case 's':
				stat = true;
				file = optarg;
				break;
			case 'r':
				render = true;
				file = optarg;
				break;
			case 'a':
				update_all = true;
				break;
		}
	}

    // if there are remaining arguments -> the page name
	if(optind < argc) {
		file = argv[optind];
	}
	for(int i = optind + 1; i < argc; i++) {
		file.append("-");
		file.append(argv[i]);
	}
}