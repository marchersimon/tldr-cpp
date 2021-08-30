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
	bool precedingSpace = true;
	for(const Token & token : line) {
		str.append(token.orig);
	}
	return str;
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
		formatPage(page);
		TPage tpage;
		tpage.namelen = page.name.length();
		tpage.name = tokenizeBySpace(page.name); // git local-commits -> {"git", "local-commits"}
		tpage.platform = page.platform;
		std::stringstream descrStream(page.description);
		while(descrStream.good()) {
			string nextLine;
			std::getline(descrStream, nextLine);
			tpage.descr.push_back(tokenize(nextLine));
		}
		tpage.descr.pop_back(); // since page.description ends with \n, getline will create an empty element at the end

		// ignoring example descriptions for now, because they kind of mess stuff up
		/*for(const auto & example : page.examples) {
			TExample texample;
			texample.descr = tokenize(example.description);
			texample.command = example.command;
			tpage.examples.push_back(texample);
		}*/
		for(Line & line : tpage.descr) {
			for(Token & token : line) {
				token.makeSearchable();
			}
		}
		/*for(TExample & example : tpage.examples) {
			for(Token & token : example.descr) {
				token.makeSearchable();
			}
		}*/
		pages.push_back(tpage);
	}

	return pages;
}

void formatPage(Page & page) {
    // Remove "# " in front of page name
	page.name.erase(0, 2);

    // Remove "> " in the original descrition
	int pos = page.description.find("> ");
	while (pos != string::npos) {
		page.description.erase(pos, 2);
		pos = page.description.find("> ");
	}
	
	// Remove "- " in all example descriptions
	for(Example & example : page.examples) {
		example.description.erase(0, 2);
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

    // Remove all backtichs
	pos = page.description.find('`');
	while(pos != string::npos) {
		page.description.erase(pos, 1);
		pos = page.description.find(pos, '`');
	}
	for(Example & example : page.examples) {
		pos = example.description.find('`');
		while(pos != string::npos) {
			example.description.erase(pos, 1);
			pos = example.description.find(pos, '`');
		}
	}
}

void printMatches(vector<TPage> & tpages) {

	int maxNamelen = 0;
	for(TPage & tpage : tpages) {
		if(tpage.namelen > maxNamelen) {
			maxNamelen = tpage.namelen;
		}
	}
	maxNamelen += 2;

	for(const auto & tpage : tpages) {
		std::cout.precision(3);
		std::cout.fill(' ');
		std::cout << std::left << tpage.score << " ";
		std::cout << tpage.name.str();
		std::cout << string(maxNamelen - tpage.namelen, ' ') << tpage.descr[0].str() << std::endl;
		for(int i = 1; i < tpage.descr.size(); i++) {
			std::cout << string(maxNamelen + 6, ' ') << tpage.descr[i].str() << std::endl;
		}
	}
}

void highlightMatches(vector<TPage> & tpages, vector<string> searchTerms) {
	for(auto & tpage : tpages) {
		for(Token & token : tpage.name) {
			for(string & searchTerm : searchTerms) {
				int pos = token.orig.find(searchTerm);
				if(pos != string::npos) {
					token.orig.insert(pos + searchTerm.length(), global::color::dfault);
					token.orig.insert(pos, global::color::foundMatch);
				}
			}
		}
		for(Line & line : tpage.descr) {
			for(Token & token : line) {
				for(string & searchTerm : searchTerms) {
					if(token.srch == searchTerm) {
						token.orig.insert(0, global::color::foundMatch);
						token.orig.append(global::color::dfault);
					}
				}
			}
		}
	}
}

vector<double> getInverseDocumentFrequency(vector<TPage> & tpages, std::vector<string> & SVMset) {
	vector<double> IDF;
	
	for(const string & word : SVMset) {
		int docsContainingWord = 0;
		for(int i = 0; i < tpages.size(); i++) {
			for(Line & descr : tpages[i].descr) {
				for(Token & token : descr) {
					if(word == token.srch) {
						docsContainingWord++;
						goto nextPage;
					}
				}
			}
			for(auto & example : tpages[i].examples) {
				for(Token & token : example.descr) {
					if(word == token.srch) {
						docsContainingWord++;
						goto nextPage;
					}
				}
			}
			for(Token & token : tpages[i].name) {
				int pos = token.orig.find(word);
				if(pos != string::npos) {
					docsContainingWord++;
					goto nextPage;
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
			int termsInExamples = 0;
			int termCount = 0;
			for(Line & line : tpage.descr) {
				for(Token & token : line) {
					if(isalpha(token.orig[0])) {
						numberOfTerms++;
						if(token.srch == word) {
							termCount++;
						}
					}
				}
			}
			for(auto & example : tpage.examples) {
				for(auto & token : example.descr) {
					if(isalpha(token.orig[0])) {
						numberOfTerms++;
						if(token.srch == word) {
							termsInExamples++;
						}
					}
				}
			}
			for(Token & token : tpage.name) {
				int pos = token.orig.find(word);
				if(pos != string::npos) {
					termCount++;
				}
			}
			// If a term appears in the description it's 3 times as relevant
			double termFrequency = ((double)termCount * 0.75 + (double)termsInExamples * 0.25) / numberOfTerms;
			tpage.termFrequency = termFrequency;
			tpage.tfidf.push_back(termFrequency * idf[i]);
			i++;
		}
	}
}

double getDotProduct(vector<double> v1, vector<double> v2) {
	double dotProduct = 0;
	for(int i = 0; i < v1.size(); i++) {
		dotProduct += v1[i] * v2[i];
	}
	return dotProduct;
}

void find(vector<string> searchTerms) {

	vector<TPage> tpages = getAllPages();

	// stem all search terms
	for(string & searchTerm : searchTerms) {
		stem(searchTerm);
	}

	for(auto & tpage : tpages) {
		
		for(Token & token : tpage.name) {
			for(string & searchTerm : searchTerms) {
				int pos = token.orig.find(searchTerm);
				if(pos != string::npos) {
					tpage.hasMatch = true;
					goto nextPage;
				}
			}
		}
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
		nextPage:;
	}


	for(int i = tpages.size() - 1; i >= 0; i--) {
		if(!tpages[i].hasMatch) {
			tpages.erase(tpages.begin() + i);
		}
	}

	if(tpages.empty()) {
		if(!global::opts::platform.empty()) {
			std::cout << "Nothing found. Try being a bit less specific" << std::endl;
			return;
		} else {
			std::cout << "Nope. Nothing." << std::endl;
			return;
		}
	}

	vector<double> IDF = getInverseDocumentFrequency(tpages, searchTerms);

	for(int i = 0; i < searchTerms.size(); i++) {
		std::cout << searchTerms[i] << ": " << IDF[i] << std::endl;
	}
	

	calculateTFIDF(tpages, searchTerms, IDF);
/*
	for(auto & tpage : tpages) {
		vector<double> diff;
		for(int i = 0; i < tpage.tfidf.size(); i++) { 
			diff.push_back(IDF[i] - tpage.tfidf[i]);
		}
		double a = 0;
		for(double val : diff) {
			a += val * val;
		}
		double absIDF = std::sqrt(a);
		tpage.score = 1 / absIDF;
	}*/
	for(auto & tpage : tpages) {
		double dot = getDotProduct(tpage.tfidf, IDF);
		double a = 0;
		for(double val : tpage.tfidf) {
			a += val * val;
		}
		double absPage = std::sqrt(a);
		a = 0;
		for(double val : IDF) {
			a += val * val;
		}
		double absIDF = std::sqrt(a);
		tpage.score = dot / (absIDF * absPage);

	}

	for(auto & tpage : tpages) {
		for(const Token & token : tpage.name) {
			// if the token isn't a word
			if(!isalpha(token.orig[0])) {
				continue;
			}
			for(string & searchTerm : searchTerms) {
				if(searchTerm == token.orig) {
					tpage.score *= 1.25;
				// if one of it starts with the search term
				} else if(token.orig.starts_with(searchTerm)) {
					tpage.score *= 1.20;
				} else {
					int pos = token.orig.find(searchTerm);
					// if one of it contains the search term
					if(pos != string::npos) {
						tpage.score *= 1.15;
					} 
					// else reduce the score a bit
					else {
						tpage.score *= 0.90;
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

	printMatches(tpages);
}

bool isStopWord(const string word) {

	static vector<string> stopWords = { // from https://gist.github.com/sebleier/554280
		"it", "its", "itself",	"which", "this", "that", "is", "are", "have", "has",
		"having", "do", "does", "did", "doing", "a", "an", "the", "and", "but", "if", "or", "because", "as", "until", "while", "of", "at", "by",
		"for", "with", "about", "against", "between", "into", "through", "during", "before", "after", "above", "below", "to", "from", "up",
		"down", "in", "out", "on", "off", "over", "under", "again", "further", "then", "once", "here", "there", "when", "where", "why", "how",
		"all", "any", "both", "each", "few", "more", "most", "other", "some", "such", "no", "nor", "not", "only", "own", "same", "so", "than",
		"too", "very", "s", "t", "can", "will", "just", "don", "should", "now"
	};

	for(const auto & stopWord : stopWords) {
		if(word == stopWord) {
			return true;
		}
	}
	return false;
}


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

vector<string> tokenizeBySpace (string s) { // used for page names
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