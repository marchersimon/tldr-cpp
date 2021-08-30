#include "find.h"

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

void formatPage(TokenizedPage & tpage) {
    // Remove "# " in front of page name
    tpage.name.remove(0);
	tpage.name.remove(0);

    // Remove "> " in the original descrition
	for(Line & line : tpage.descr) {
		line.remove(0); // ">" is first token
	}
	
	// Remove "- " in all example descriptions
	for(SearchableExample & example : tpage.examples) {
		example.descr.remove(0);
	}

	// Remove More information link in case there is one
	if(tpage.descr.back().at(0).orig == "More" && tpage.descr.back().at(2).orig == "information") {
		tpage.descr.pop_back();
	}

	// Remove "See also" notes
	for(int i = 0; Line & line : tpage.descr) {
		if(line.at(0).orig == "See" && line.at(1).orig == "also") {
			tpage.descr.erase(tpage.descr.begin() + i);
		}
		i++;
	}

    // Remove all backtichs
	/*
	for(Line & line : tpage.descr) {
		for(int i = line.size() - 1; i >= 0; i--) {
			if(line.at(i).orig[0] == '`') {
				line.remove(i);
			}
		}
	}
	for(SearchableExample & example : tpage.examples) {
		for(int i = example.descr.size() - 1; i >= 0; i--) {
			if(example.descr.at(i).orig[0] == '`') {
				example.descr.remove(i);
			}
		}
	}
	*/
	// copy the original descriptions to the searchbale ones
	// Then remove all punctuations and turn everything lowercase
	for(Line & line : tpage.descr) {
		for(Token & token : line) {
			if(ispunct(token.orig[0])) { // if it's a punctuation it only has one character
				token.srch = "";
			} else {
				for(const char & c : token.orig) {
					token.srch += std::tolower(c);
				}
			}
		}
	}
	for(SearchableExample & example : tpage.examples) {
		for(Token & token : example.descr) {
			if(ispunct(token.orig[0])) { // if it's a punctuation it only has one character
				token.srch = "";
			} else {
				for(const char & c : token.orig) {
					token.srch += std::tolower(c);
				}
			}
		}
	}
}

vector<TokenizedPage> getAllPages() {
	vector<TokenizedPage> pages;

	for(const auto & entry : std::filesystem::recursive_directory_iterator(global::tldrPath + "pages/")) {// needs C++17
		if(entry.is_directory()) {
			continue;
		}
		std::ifstream fileStream(entry.path());
		string file, temp;
		while(std::getline(fileStream, temp)) {
			file += temp + '\n';
		}
		fileStream.close();
		Page page(file);
		TokenizedPage tpage;
		tpage.name = tokenizeBySpace(page.name);
		tpage.platform = page.platform;
		std::stringstream descrStream(page.description);
		while(descrStream.good()) {
			string nextLine;
			std::getline(descrStream, nextLine);
			tpage.descr.push_back(tokenize(nextLine));
		}
		tpage.descr.pop_back(); // since page.description ends with \n, getline will create an empty element at the end
		for(const auto & example : page.examples) {
			SearchableExample searchExample;
			searchExample.descr = tokenize(example.description);
			searchExample.command = example.command;
			tpage.examples.push_back(searchExample);
		}
		formatPage(tpage);
		pages.push_back(tpage);
	}

	return pages;
}

string join(vector<string> vect) {
	string s;
	for(string str : vect) {
		s.append(str + " ");
	}
	s.pop_back(); // remove last space avain
	return s;
}

Token::Token(string origWord) {
    orig = origWord;
}

Line::Line(vector<string> origLine) {
	for(const string & word : origLine) {
		line.push_back(word);
	}
}

void printMatches(const vector<TokenizedPage> & tpages) {

	for(const auto & tpage : tpages) {
		printf("%3lf ", tpage.score);
		std::cout << tpage.name.str() << std::endl;
		for(const Line & line : tpage.descr) {
			std::cout << "    " << line.str() << std::endl;
		}
	}
}

void highlightMatches(vector<Match> & matches) {
	for(auto & match : matches) {
		for(auto & highlight : match.descrMatchedAt) {
			match.descr.at(highlight[0]).at(highlight[1]).orig.insert(0, global::color::foundMatch);
			match.descr.at(highlight[0]).at(highlight[1]).orig.append(global::color::dfault);
		}
	}
}

vector<double> getInverseDocumentFrequency(vector<TokenizedPage> & tpages, std::vector<string> & SVMset) {
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
	for(TokenizedPage & tpage : tpages) {
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
				if(token.orig == word) {
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

	vector<TokenizedPage> tpages = getAllPages();


	for(auto & tpage : tpages) {
		// stem all pages
		for(Line & line : tpage.descr) {
			for(Token & token : line) {
				if(!token.srch.empty()) {
					stem(token.srch);
				}
				
			}
		}	
		for(SearchableExample & example : tpage.examples) {
			for(Token & token : example.descr) {
				if(!token.srch.empty()) {
					stem(token.srch);
				}
			}
		}

		 //remove all stop words
		for(auto & descrLine : tpage.descr) {
			removeStopWords(descrLine);
		}
		for(auto & example : tpage.examples) {
			removeStopWords(example.descr);
		}
		
	}

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

	std::sort(tpages.begin(), tpages.end(), [](const TokenizedPage & x, const TokenizedPage & y) {return x.score > y.score;});
	/*for(auto & tpage : tpages) {
		std::cout << tpage.score << " " << tpage.name.str() << std::endl;
	}*/
	
	printMatches(tpages);

	/*
	Match scoring system:
		If the page name matches exactly, the page get's 1000 points
		If the page name contains the search term it get's 500 points
		If a matched page name has subcommands, only the 3 highest scored are shown

		For each occurence in the first description line the page gets 100 points
		For each occurence in the second description line the page gets 50 points
		For each occurence in an example description, the page get's 50 points
		Only the 3 highest scored examples of a page are shown
	*/
	vector<Match> matches;

	for(TokenizedPage & tpage : tpages) {
		Match match;
		for(const string & searchTerm : searchTerms) {
			// check for matches in the page name
			for(const Token & token : tpage.name) {
				// if the token isn't a word
				if(token.orig[0] == ' ') {
					continue;
				}
				// if the search term matches exactly one of the words in the name
				if(searchTerm == token.orig) {
					match.matched = true;
					tpage.score += 1000 / tpage.name.line.size();
				// if one of it starts with the search term
				} else if(token.orig.starts_with(searchTerm)) {
					match.matched = true;
					match.score += 750 / tpage.name.line.size();
				} else {
					int pos = token.orig.find(searchTerm);
					// if one of it contains the search term
					if(pos != string::npos) {
						match.matched = true;
						match.score += 500 / tpage.name.line.size();
					} 
					// else reduce the score a bit
					else {
						match.score *= 0.75;
					}
				}
			}
		}
		if(match.matched) {
			match.name = tpage.name.str();
			match.nameLen = match.name.length();
			match.descr = tpage.descr;
			matches.push_back(match);
		}
	}

	/*
	for(auto & tpage : tpages) {
		Match match;
		for(string searchTerm : searchTerms) {
			if(tpage.platform == searchTerm) {
				match.score += 500;
			}
			int pos = tpage.name.find(searchTerm);
			if(pos != string::npos) {
				if(match.name.empty()) {
					match.name = tpage.name;
				}
				match.nameLen = tpage.name.length();
				match.name.insert(pos + searchTerm.length(), global::color::dfault);
				match.name.insert(pos, global::color::foundMatch);
				match.descr.push_back(tpage.descr[0]);
				if(tpage.name == searchTerm) {
					match.score += 1000;
				} else if (tpage.name.starts_with(searchTerm) || tpage.name[pos - 1] == ' ') {
					match.score += 750;
				} else {
					match.score += 500;
				}
			} else {
				match.score -= 500;
			}
			for(int i = 0; i < tpage.descr.size(); i++) {
				for(int j = 0; j < tpage.descr[i].size(); j++) {
					if(tpage.descr[i].at(j).srch == searchTerm) {
						if(match.name.empty()) {
							match.name = tpage.name;
							match.nameLen = tpage.name.length();
						}
						if(match.descr.size() < i + 1) {
							match.descr.clear();
							for(int k = 0; k < i + 1; k++) {
								match.descr.push_back(tpage.descr[k]);
							}
						}
						match.descrMatchedAt.push_back({i, j});
						match.score += 50 * (2 - i);
					}
				}
			}
		}
		if(!match.name.empty()) {
			matches.push_back(match);
		}
	}*/

	std::sort(matches.begin(), matches.end(), [](const Match & x, const Match & y) {return x.score > y.score;});
	//highlightMatches(matches);
	//printMatches(matches);
}

void removeStopWords(Line & line) {

	static vector<string> stopWords = { // from https://gist.github.com/sebleier/554280
		"it", "its", "itself",	"which", "this", "that", "is", "are", "have", "has",
		"having", "do", "does", "did", "doing", "a", "an", "the", "and", "but", "if", "or", "because", "as", "until", "while", "of", "at", "by",
		"for", "with", "about", "against", "between", "into", "through", "during", "before", "after", "above", "below", "to", "from", "up",
		"down", "in", "out", "on", "off", "over", "under", "again", "further", "then", "once", "here", "there", "when", "where", "why", "how",
		"all", "any", "both", "each", "few", "more", "most", "other", "some", "such", "no", "nor", "not", "only", "own", "same", "so", "than",
		"too", "very", "s", "t", "can", "will", "just", "don", "should", "now"
	};

	for(int i = line.size() - 1; i >= 0; i--) {
		for(const auto & stopWord : stopWords) {
			if(line.at(i).srch == stopWord) {
				line.at(i).srch = "";
			}
		}
	}
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