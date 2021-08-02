#pragma once

#include <iostream>

using std::string;

namespace global {
	inline string tldrPath;
	inline string HOME;
	namespace color {
		inline string dfault = "\033[0m";
		inline string title = "\033[1;38;2;200;50;50m"; // bold;set-fg-color;rgb;<r>;<g>;<b>m
		inline string backtick = "\033[38;5;110m";
		inline string command = "\033[38;2;200;100;150m";
		inline string token = "\033[38;2;40;215;66m";
	}
}