#pragma once

#include <getopt.h>
#include <string>
#include <vector>
#include <sstream>

#include "global.h"

namespace opts {

	void parseLanguages(char* optarg);

	void parse(int argc, char *argv[]);
}
