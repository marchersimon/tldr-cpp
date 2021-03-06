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
		extern bool find;
		extern vector<string> search_terms;
		extern bool findOverrideDefaults;
		extern bool name;
		extern bool description;
		extern bool examples;
		extern bool stem;
		extern bool sub;
		extern bool destroy;
		extern bool pr;
		extern string prNumber;
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
		extern string foundMatch;
		extern string pageCreated;
		extern string pageModified;
	}

	void init();
}