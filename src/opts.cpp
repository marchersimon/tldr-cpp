#include "opts.h"

using namespace global::opts;

vector<string> arguments;
uint index;

/*
Parses a comma-separated list of languages like "en,fr,it" and puts them in a list
*/
void parseLanguages(string lang) {
	languages.clear();
	std::stringstream languagelist(lang);
	while(languagelist.good()) {
		string nextLanguage;
		std::getline(languagelist, nextLanguage, ',');
		for(char & c : nextLanguage) {
			if(ispunct(c) && c != '_') {
				throw std::runtime_error("Invalid language: " + nextLanguage + ". Only letters and underscores are allowed");
			}
		}
		if(nextLanguage.size() < 2) {
			throw std::runtime_error("Language codes have to be at least two letters long.");
		} else {
			nextLanguage[0] = tolower(nextLanguage[0]);
			nextLanguage[1] = tolower(nextLanguage[1]); // DE to de
			if(nextLanguage.size() == 5) {
				nextLanguage[3] = toupper(nextLanguage[3]);
				nextLanguage[4] = toupper(nextLanguage[4]); // zh_tw to zh_TW
			}
		}
		languages.push_back(nextLanguage);
	}
}

/*
Parses the rest of the arguments and puts them into a list
*/
void parseSearchTerms() {
	index++;
	for(; index < arguments.size(); index++) {
		std::stringstream argument(arguments[index]);
		while(argument.good()) {
			string nextArgument;
			std::getline(argument, nextArgument, ' ');
			search_terms.push_back(nextArgument);
		}
	}
	if(search_terms.empty()) {
		throw std::runtime_error(arguments[index - 1] + " needs at least one argument");
	}
}

vector<string> tokenizeOpts(const int & argc, char* argv[]) {
	vector<string> arguments;
	for(int i = 1; i < argc; i++) {
		string argument = argv[i];
		if(argument.starts_with("-") && !argument.starts_with("--")) {
			for(uint j = 1; j < argument.size(); j++) {
				arguments.push_back(string("-") + argument[j]);
			}
		} else {
			arguments.push_back(argument);
		}
	}
	return arguments;
}

bool isArgument(vector<string> flags) {
	for(const string & flag : flags) {
		if(arguments[index] == flag) {
			return true;
		}
	}
	return false;
}

void parsePage() {
	uint i;
	for(i = index; i < arguments.size(); i++) {
		if(arguments[i].starts_with("-")) {
			break;
		} 
		file += arguments[i];
		file += "-";
	}
	file.pop_back();
	index += i - index - 1;
}

string nextArg() {
	index++;
	if(index >= arguments.size()) {
		throw std::runtime_error(arguments[index - 1] + " needs an argument");
	} else {
		return arguments[index];
	}
}

/*
Parses all command line argument and saves them to global::opts::...
*/
void opts::parse(const int & argc, char* argv[]) {

	arguments = tokenizeOpts(argc, argv);

	bool expectedPage = true;
	bool gotPage = false;

	for(index = 0; index < arguments.size(); index++) {
		#define arg arguments[index]
		if(isArgument({"-h", "--help"})) {
			help = true;
			expectedPage = false;
		} else if(isArgument({"-u", "--update"})) {
			update = true;
			expectedPage = false;
		} else if(isArgument({"-l", "--language"})) {
			parseLanguages(nextArg());
		} else if(isArgument({"-p", "--platform"})) {
			platform = nextArg();
		} else if(isArgument({"-v", "--verbose"})) {
			verbose = true;
		} else if(isArgument({"-s", "--stat"})) {
			stat = true;
		} else if(isArgument({"--raw"})) {
			raw = true;
		} else if(isArgument({"-r", "--render"})) {
			expectedPage = false;
			render = true;
			file = nextArg();
		} else if(isArgument({"-a", "--all"})) {
			update_all = true;
		} else if(isArgument({"-f", "--find"})) {
			expectedPage = false;
			find = true;
			if(!help) {
				parseSearchTerms();
			}
		} else if(isArgument({"--name"})) {
			name = true;
			findOverrideDefaults = true;
		} else if(isArgument({"--description"})) {
			description = true;
			findOverrideDefaults = true;
		} else if(isArgument({"--examples"})) {
			examples = true;
			findOverrideDefaults = true;
		} else if(isArgument({"--stem"})) {
			stem = true;
		} else if(isArgument({"--sub"})) {
			sub = true;
		#ifdef _MANUAL_INSTALL_
		} else if(isArgument({"--destroy"})) {
			destroy = true;
			expectedPage = false;
		#endif
		} else if(isArgument({"--pr"})) {
			pr = true;
			expectedPage = false;
			prNumber = nextArg();
		} else {
			if(arg[0] == '-') {
				throw std::runtime_error(arg + ": Unknown argument");
			} else {
				if(gotPage) {
					throw std::runtime_error("Unexpected argument: " + arg);
				}
				parsePage();
				gotPage = true;
			}
		}
	}

	if(expectedPage && !gotPage) {
		throw std::runtime_error("Please specify a page name");
	}
}