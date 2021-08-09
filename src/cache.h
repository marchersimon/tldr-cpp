#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <filesystem>
#include <fstream>

#include "global.h"
#include "page.h"

namespace cache {

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

	void verify();

	void findPlatforms();

	Page getPage(string name);

	Page getPageFromPath(string path);

	Index getFromIndex(string name);

	void stat(string name);
}
