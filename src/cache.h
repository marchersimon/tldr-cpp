#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>

#include "global.h"
#include "opts.h"
#include "page.h"

namespace cache {
	class Platform {
		public:
			string name;
			int numberOfPages = 0;
	};

	class Structure {
		public:
			std::vector<Platform> platforms;
			void sortPlatforms();
	};

	void init();

	Structure check();

	Page getPage(string name, std::vector<Platform> platforms);
}
