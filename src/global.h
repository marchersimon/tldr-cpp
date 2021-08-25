#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace global {

	extern vector<string> platforms;

	namespace opts {
		extern bool help;
		extern bool update;
		extern string file;
		extern vector<string> languages;
		extern bool verbose;
		extern string platform;
		extern bool raw;
		extern bool stat;
		extern bool render;
		extern bool update_all;
	}

	extern string tldrPath;
	extern string HOME;

	namespace color {
		extern string dfault;
		extern string title;
		extern string backtick;
		extern string command;
		extern string token;
		extern string stat_checkmark;
	}

	void init();
}