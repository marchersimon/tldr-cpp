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

	class Index {
		public:
			class Target {
				public:
					Target(string platform, string language);
					string platform;
					string language;
			};

		std::vector<Target> targets;
		bool contains(Target target);
	};

	void init();

	Structure check();

	Page getPage(string name, std::vector<Platform> platforms);

	Index getFromIndex(string name);
}
