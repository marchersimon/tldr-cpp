#include "find.h"

// class Line
//======================

Line::Line(vector<string> origLine) {
	for(const string & word : origLine) {
		line.push_back(word);
	}
}

Token& Line::at(int n) {
	return line.at(n);
}

void Line::remove(int n) {
	line.erase(line.begin() + n);
}

// With begin() and end() I can use for(Token & t : line), instead of for(Token & t : line.line)
vector<Token>::iterator Line::begin() {
	return line.begin();
}
vector<Token>::iterator Line::end() {
	return line.end();
}

int Line::size() {
	return line.size();
}

string Line::str() const {
	string str;
	for(const Token & token : line) {
		str.append(token.orig);
	}
	return str;
}

string Line::sstr() const { // get string from searchable variant
	string sstr;
	for(const Token & token : line) {
		sstr.append(token.srch);
		if(isalpha(token.srch[0])) {
			sstr.append(" ");
		}
	}
	return sstr;
}

// class Token
//======================l

Token::Token(string origWord) {
    orig = origWord;
}

void Token::makeSearchable() {

	if(!isalpha(orig[0])) {
		srch = "";
		return;
	}
	string token = orig;
	std::transform(token.begin(), token.end(), token.begin(), ::tolower);

	if(isStopWord(token)) {
		srch = "";
		return;
	}
	stem(token);
	srch = token;
}

//===============================

vector<TPage> getAllPages() {
	
	vector<TPage> pages;

	for(const auto & entry : std::filesystem::recursive_directory_iterator(global::tldrPath + "pages/")) {// needs C++17
		if(entry.is_directory()) {
			continue;
		}
		if(!global::opts::platform.empty()) {
			int endpos = entry.path().string().rfind('/');
			int startpos = entry.path().string().rfind('/', endpos - 1);
			string platform = entry.path().string().substr(startpos + 1, endpos - startpos -1);
			if(platform != "common" && platform != global::opts::platform) {
				continue;
			}
		}
		std::ifstream fileStream(entry.path());
		string file, temp;
		while(std::getline(fileStream, temp)) {
			file += temp + '\n';
		}
		fileStream.close();
		Page page(file);
		// remove unneccessary parts
		formatPage(page);
		TPage tpage;
		tpage.namelen = page.name.length();
		tpage.name = tokenizeBySpace(page.name); // git local-commits -> {"git", "local-commits"}
		tpage.platform = page.platform; 
		// get all description lines
		std::stringstream descrStream(page.description);
		if(searchDescr) {
			while(descrStream.good()) {
				string nextLine;
				std::getline(descrStream, nextLine);
				tpage.descr.push_back(tokenize(nextLine));
			}
			tpage.descr.pop_back(); // since page.description ends with \n, getline will create an empty element at the end
		} else {
			// only get the first line if descriptions aren't seached for
			string nextLine;
			std::getline(descrStream, nextLine);
			tpage.descr.push_back(tokenize(nextLine));
		}
		// get all examples
		if(searchExamples) {
			for(const auto & example : page.examples) {
				TExample texample;
				texample.descr = tokenize(example.description);
				texample.command = example.command;
				tpage.examples.push_back(texample);
			}
		}
		// create the searchable variant of all descriptions
		if(searchDescr) {
			for(Line & line : tpage.descr) {
				for(Token & token : line) {
					token.makeSearchable();
				}
			}
		}
		if(searchExamples) {
			for(TExample & example : tpage.examples) {
				for(Token & token : example.descr) {
					token.makeSearchable();
				}
			}
		}
		pages.push_back(tpage);
	}

	return pages;
}

void formatPage(Page & page) {
    // Remove "# " in front of page name
	page.name.erase(0, 2);

	std::size_t pos;
	if(searchDescr) {
		// Remove "> " in the original descrition
		pos = page.description.find("> ");
		while (pos != string::npos) {
			page.description.erase(pos, 2);
			pos = page.description.find("> ");
		}

		// Remove More information link in case there is one
		pos = page.description.find("More information: <");
		if(pos != string::npos) {
			page.description.resize(pos);
		}

		// Remove "See also" notes
		pos = page.description.find("See also ");
		if(pos != string::npos) {
			int EOLpos = page.description.find(pos, '\n');
			page.description.erase(pos, EOLpos - pos);
		}
	} else {
		// else only remove the > in the first line
		page.description.erase(0, 2);
	}
	
	/*
	if(searchExamples) {
		// Remove "- " in all example descriptions
		for(Example & example : page.examples) {
			example.description.erase(0, 2);
		}
	}*/

    // Remove all backtichs
	if(searchDescr) {
		pos = page.description.find('`');
		while(pos != string::npos) {
			page.description.erase(pos, 1);
			pos = page.description.find('`', pos);
		}
	}
	if(searchExamples) {
		for(Example & example : page.examples) {
			pos = example.description.find('`');
			while(pos != string::npos) {
				example.description.erase(pos, 1);
				pos = example.description.find('`', pos);
			}
		}
	}
}

void printMatches(vector<TPage> & tpages) {

	int maxNamelen = 0;
	int statlen = 0;
	for(TPage & tpage : tpages) {
		if(tpage.namelen > maxNamelen) {
			maxNamelen = tpage.namelen;
		}
	}
	maxNamelen += 2;

	if(global::opts::verbose) {
		statlen = 6;
	}

	for(const auto & tpage : tpages) {
		if(global::opts::verbose) {
			std::cout.precision(3);
			std::cout.fill('0');
			std::cout << std::left << std::setw(5) << tpage.score << " ";
		}
		std::cout << tpage.name.str();
		std::cout << string(maxNamelen - tpage.namelen, ' ') << tpage.descr[0].str() << std::endl;
		if(searchDescr) {
			for(uint i = 1; i < tpage.descr.size(); i++) {
				std::cout << string(maxNamelen + statlen, ' ') << tpage.descr[i].str() << std::endl;
			}
		}
		if(searchExamples) {
			for(const TExample & example : tpage.examples) {
				std::cout << string(maxNamelen + statlen + 2, ' ') << example.descr.str() << std::endl;
				std::cout << string(maxNamelen + statlen + 4, ' ') << example.command << std::endl;
			}
		}
	}
}

void highlightMatches(vector<TPage> & tpages, vector<string> searchTerms) {
	for(auto & tpage : tpages) {
		if(searchName) {
			for(Token & token : tpage.name) {
				for(string & searchTerm : searchTerms) {
					std::size_t pos = token.orig.find(searchTerm);
					if(pos != string::npos) {
						token.orig.insert(pos + searchTerm.length(), global::color::dfault);
						token.orig.insert(pos, global::color::foundMatch);
					}
				}
			}
		}
		if(searchDescr) {
			for(Line & line : tpage.descr) {
				bool hasMatch = false;
				for(Token & token : line) {
					for(string & searchTerm : searchTerms) {
						if(token.srch == searchTerm) {
							token.orig.insert(0, global::color::foundMatch);
							token.orig.append(global::color::dfault);
							hasMatch = true;
						}
					}
				}
				tpage.matchedDescriptionLines.push_back(hasMatch);
			}
		}
		if(searchExamples) {
			for(TExample & example : tpage.examples) {
				bool hasMatch = false;
				for(Token & token : example.descr) {
					for(string & searchTerm : searchTerms) {
						if(token.srch == searchTerm) {
							token.orig.insert(0, global::color::foundMatch);
							token.orig.append(global::color::dfault);
							hasMatch = true;
						}
					}
				}
				tpage.matchedExamples.push_back(hasMatch);
			}
		}
	}
}

void removeNonMatchedLines(vector<TPage> & tpages) {

	for(TPage & tpage : tpages) {
		if(searchDescr) {
			for(int i = tpage.descr.size() - 1; i > 0; i--) {
				if(!tpage.matchedDescriptionLines[i]) {
					tpage.descr.erase(tpage.descr.begin() + i);
				}
			}
		}
		if(searchExamples) {
			for(int i = tpage.examples.size() - 1; i >= 0; i--) {
				if(!tpage.matchedExamples[i]) {
					tpage.examples.erase(tpage.examples.begin() + i);
				}
			}
		}
	}	
}

vector<double> getIDF(vector<TPage> & tpages, std::vector<string> & terms) {
	vector<double> IDF;
	
	for(const string & word : terms) {
		int docsContainingWord = 0;
		for(uint i = 0; i < tpages.size(); i++) {
			if(searchDescr) {
				for(Line & descr : tpages[i].descr) {
					for(Token & token : descr) {
						if(word == token.srch) {
							docsContainingWord++;
							goto nextPage;
						}
					}
				}
			}
			if(searchExamples) {
				for(auto & example : tpages[i].examples) {
					for(Token & token : example.descr) {
						if(word == token.srch) {
							docsContainingWord++;
							goto nextPage;
						}
					}
				}
			}
			if(searchName) {
				for(Token & token : tpages[i].name) {
					std::size_t pos = token.orig.find(word);
					if(pos != string::npos) {
						docsContainingWord++;
						goto nextPage;
					}
				}
			}
			nextPage:;
		}
		double idf;
		if(docsContainingWord == 0) {
			idf = 0;
		} else {
			double tmp = (double)tpages.size() / (docsContainingWord + 1);
			idf = log(tmp) + 1;
		}
		IDF.push_back(idf);
	}
	return IDF;
}

void calculateTFIDF(auto & tpages, std::vector<string> SVMset, vector<double> idf) {
	for(TPage & tpage : tpages) {
		for(int i = 0; const string & word : SVMset) {
			int numberOfTerms = 0;
			int matchesInExamples = 0;
			int matchesInDescription = 0;
			if(searchDescr) {
				for(Line & line : tpage.descr) {
					for(Token & token : line) {
						if(isalpha(token.orig[0])) {
							numberOfTerms++;
							if(token.srch == word) {
								matchesInDescription++;
							}
						}
					}
				}
			}
			if(searchExamples) {
				for(auto & example : tpage.examples) {
					for(auto & token : example.descr) {
						if(isalpha(token.orig[0])) {
							numberOfTerms++;
							if(token.srch == word) {
								matchesInExamples++;
							}
						}
					}
				}
			}
			if(searchName) {
				for(Token & token : tpage.name) {
					numberOfTerms++;
					std::size_t pos = token.orig.find(word);
					if(pos != string::npos) {
						matchesInDescription++;
					}
				}
			}
			// If a term appears in the description it's 3 times as relevant
			double termFrequency = ((double)matchesInDescription * 0.75 + (double)matchesInExamples * 0.25) / numberOfTerms;
			tpage.termFrequency = termFrequency;
			tpage.tfidf.push_back(termFrequency * idf[i]);
			i++;
		}
	}
}

double getDotProduct(vector<double> v1, vector<double> v2) {
	double dotProduct = 0;
	for(uint i = 0; i < v1.size(); i++) {
		dotProduct += v1[i] * v2[i];
	}
	return dotProduct;
}

void find(vector<string> searchTerms) {


	// convert search terms to lowercase
	for(auto & searchTerm : searchTerms) {
		std::transform(searchTerm.begin(), searchTerm.end(), searchTerm.begin(),
    		[](unsigned char c){ return std::tolower(c); });
	}

	// stem all search terms
	for(string & searchTerm : searchTerms) {
		stem(searchTerm);
	}
	
	vector<TPage> tpages = getAllPages();

	// check if a page has at least one match, else it will be removed directly
	for(auto & tpage : tpages) {
		if(searchName) {
			for(Token & token : tpage.name) {
				for(string & searchTerm : searchTerms) {
					std::size_t pos = token.orig.find(searchTerm);
					if(pos != string::npos) {
						tpage.hasMatch = true;
						goto nextPage;
					}
				}
			}
		}
		if(searchDescr) {
			for(Line & line : tpage.descr) {
				for(Token & token : line) {
					for(string & searchTerm : searchTerms) {
						if(searchTerm == token.srch) {
							tpage.hasMatch = true;
							goto nextPage;
						}
					}
				}
			}
		}
		if(searchExamples) {
			for(auto & example : tpage.examples) {
				for(auto & token : example.descr) {
					for(string & searchTerm : searchTerms) {
						if(searchTerm == token.srch) {
							tpage.hasMatch = true;
							goto nextPage;
						}
					}
				}
			}
		}
		nextPage:;
	}

	// remove pages without match
	for(int i = tpages.size() - 1; i >= 0; i--) {
		if(!tpages[i].hasMatch) {
			tpages.erase(tpages.begin() + i);
		}
	}

	if(tpages.empty()) {
		if(!global::opts::platform.empty() || global::opts::findOverrideDefaults) {
			std::cout << "Nothing found. Try being a bit less specific" << std::endl;
			return;
		} else {
			std::cout << "Nope. Nothing." << std::endl;
			return;
		}
	}

	vector<double> IDF;
	// If I'm only searching for the search term, the IDF value won't correctly represent the importance.
	// In the same way, if I'm only searching for 1 term, the vector space will only have one axis and therefore the angle between them will be the same
	if(!searchNameOnly && searchTerms.size() > 1) {
		IDF = getIDF(tpages, searchTerms);
	} else {
		// initialize the score with 1
		for(uint i = 0; i < searchTerms.size(); i++) {
			IDF.push_back(1);
		}
	}
	

	// print the weight/importance of each search term
	if(global::opts::verbose) {
		for(uint i = 0; i < searchTerms.size(); i++) {
			std::cout << searchTerms[i] << ": " << IDF[i] << std::endl;
		}
		std::cout << std::endl;
	}

	calculateTFIDF(tpages, searchTerms, IDF);

	// calculate the cosine similarity (score) of each page
	// Or in case of only one search term, calculate the distance between the vectors
	if(!searchNameOnly && searchTerms.size() > 1) {
		for(auto & tpage : tpages) {
			double dotProduct = getDotProduct(tpage.tfidf, IDF);
			double absTFIDF = 0;
			for(double tmp : tpage.tfidf) {
				absTFIDF += tmp * tmp;
			}
			absTFIDF = std::sqrt(absTFIDF);
			double absIDF = 0;
			for(double tmp : IDF) {
				absIDF += tmp * tmp;
			}
			absIDF = std::sqrt(absIDF);
			tpage.score = dotProduct / (absIDF * absTFIDF);
		}
	} else if (!searchNameOnly) {
		for(TPage & tpage : tpages) {
			tpage.score = IDF[0] - tpage.tfidf[0];
		}
	} else {
		for(TPage & tpage : tpages) {
			tpage.score = 1;
		}
	}

	// boost each page which has matches in the name
	if(searchName) {
		for(auto & tpage : tpages) {
			for(const Token & token : tpage.name) {
				// if the token isn't a word
				if(!isalpha(token.orig[0])) {
					continue;
				}
				for(string & searchTerm : searchTerms) {
					// if the the search term matches exactly one word of the page name
					if(searchTerm == token.orig) {
						tpage.score *= 1.25;
					// if one of it starts with the search term
					} else if(token.orig.starts_with(searchTerm)) {
						tpage.score *= 1.20;
					} else {
						// if one of it contains the search term
						std::size_t pos = token.orig.find(searchTerm);
						if(pos != string::npos) {
							tpage.score *= 1.15;
						} 
						// if not, reduce the score a bit
						else {
							tpage.score *= 0.90;
						}
					}
				}
			}
		}
	}

	// sort matches by score
	std::sort(tpages.begin(), tpages.end(), [](const TPage & x, const TPage & y) {return x.score > y.score;});

	// highlight matched parts of the original descriptions
	highlightMatches(tpages, searchTerms);

	// only keep the 12 best matches
	if(tpages.size() > 12) {
		tpages.resize(12); 
	}

	// remove description lines and examples without a match
	removeNonMatchedLines(tpages);

	// format examples
	for(TPage & tpage : tpages) {
		for(TExample & example : tpage.examples) {
			example.command.pop_back();
			example.command.erase(0, 1);
			Page::formatTokenSyntax(example.command);
			example.command.insert(0, global::color::command);
			example.command.append(global::color::dfault);
		}
	}

	printMatches(tpages);
}

// returns true if it's a stop word and false if not
bool isStopWord(const string word) {

	// some of them could be removed, since they don't really appear in tldr pages
	static vector<string> stopWords = {
		"it", "its", "itself",	"which", "this", "that", "is", "are", "have", "has", "having", "does", "a", "an", "the", "and", "but", 
		"if", "or", "because", "as", "until", "while", "of", "at", "by", "for", "with", "about", "against", "between", "into", "through", 
		"during", "before", "after", "to", "from", "up", "down", "in", "out", "on", "off", "over", "under", "again", "further", "then", 
		"once", "there", "when", "where", "why", "how", "all", "any", "both", "each", "few", "more", "most", "other", "such", "no", "nor", 
		"not", "only", "own", "same", "so", "than", "too", "s", "t", "can", "will", "just", "don", "should", "now"
	};

	for(const auto & stopWord : stopWords) {
		if(word == stopWord) {
			return true;
		}
	}
	return false;
}

/*
This tokenizes each line, so that the words can seperately be searched for while the original structure stays intact
"This is. (an example)" -> "This", " ", "is", ". (", "an", " ", "example", " "
*/
vector<string> tokenize(string s) {
	
	vector<string> tokens;
	string token;
	bool word = true;
	if(ispunct(s[0]) || s[0] == ' ') {
		word = false;
	}
	for(const char & c : s) {
		if(ispunct(c) || c == ' ') {
			if(word) {
				tokens.push_back(token);
				token.clear();
				word = false;
			}
			token += c;
		} else {
			if(!word) {
				tokens.push_back(token);
				token.clear();
				word = true;
			}
			token += c;
		}
	}
	tokens.push_back(token); // add last token
	
	return tokens;
}

/*
Like tokenize, but only uses space as a delimiter, so that
"apt-get", doesn't become "apt", " - ", "get".
Used for page names.
*/
vector<string> tokenizeBySpace (string s) {
	vector<string> tokens;
	string token;
	for(const char & c : s) {
		if(c == ' ') {
			tokens.push_back(token);
			tokens.push_back(" ");
			token.clear();
		} else {
			token += c;
		}
	}
	tokens.push_back(token);
	return tokens;
}

/*
Print a page in tokenized and stemmed format
*/
void printStemmedPage(Page page) {
	formatPage(page);

	TPage tpage;
	tpage.name = tokenizeBySpace(page.name); // git local-commits -> {"git", "local-commits"}
	tpage.platform = page.platform; 
	// get all description lines
	std::stringstream descrStream(page.description);
	while(descrStream.good()) {
		string nextLine;
		std::getline(descrStream, nextLine);
		tpage.descr.push_back(tokenize(nextLine));
	}
	tpage.descr.pop_back(); // since page.description ends with \n, getline will create an empty element at the end

	// get all examples
	for(const auto & example : page.examples) {
		TExample texample;
		texample.descr = tokenize(example.description);
		texample.command = example.command;
		tpage.examples.push_back(texample);
	}

	// create the searchable variant of all descriptions
	for(Line & line : tpage.descr) {
		for(Token & token : line) {
			token.makeSearchable();
		}
	}
	for(TExample & example : tpage.examples) {
		for(Token & token : example.descr) {
			token.makeSearchable();
		}
	}

	std::cout << tpage.name.str() << std::endl << std::endl;
	for(const Line & descr : tpage.descr) {
		std::cout << descr.sstr() << std::endl;
	}
	std::cout << std::endl;
	for(const TExample & example : tpage.examples) {
		std::cout << example.descr.sstr() << std::endl << std::endl;
		std::cout << example.command << std::endl << std::endl;
	}
}