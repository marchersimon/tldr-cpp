#include "global.h"

namespace global {

	namespace opts {
		bool help = false;
		bool update = false;
		string file;
		vector<string> languages;
		bool verbose = false;
		string platform;
		bool raw = false;
		bool stat = false;
	}

	string tldrPath;
	string HOME;
	namespace color {
		string dfault = "\033[0m";
		string title = "\033[1;38;2;200;50;50m"; // bold;set-fg-color;rgb;<r>;<g>;<b>m
		string backtick = "\033[38;5;110m";
		string command = "\033[38;2;200;100;150m";
		string token = "\033[38;2;40;215;66m";
	}
}