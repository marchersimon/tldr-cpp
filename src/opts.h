#pragma once

#include <getopt.h>
#include <string>
#include <vector>
#include <sstream>

#include <iostream>

#include "global.h"

namespace opts {

	void parseLanguages(char *optarg);

	void parseSearchTerm(char *optarg, int argc, char *argv[]);

	void parse(int argc, char *argv[]);
}
