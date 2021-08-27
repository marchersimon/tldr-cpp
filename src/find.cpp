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
	for(const Token & token : line) {
		str.append(token.orig + " ");
	}
	return str;
}

void formatPage(TokenizedPage & tpage) {
    // Remove "# " in front of page name
    tpage.name.erase(0, 2);

    // Remove "> " in the original descrition
	for(Line & line : tpage.descr) {
		line.remove(0); // ">" is first token
	}
	
	// Remove "- " in all example descriptions
	for(SearchableExample & example : tpage.examples) {
		example.descr.remove(0);
	}

	// Remove More information link in case there is one
	if(tpage.descr.back().at(0).orig == "More" && tpage.descr.back().at(0).orig == "information") {
		tpage.descr.pop_back();
	}

    // Remove all backtichs
	for(Line & line : tpage.descr) {
		for(int i = line.size() - 1; i >= 0; i--) {
			if(line.at(i).orig == "`") {
				line.remove(i);
			}
		}
	}
	for(SearchableExample & example : tpage.examples) {
		for(int i = example.descr.size() - 1; i >= 0; i--) {
			if(example.descr.at(i).orig == "`") {
				example.descr.remove(i);
			}
		}
	}

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
		tpage.name = page.name;
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

void printMatches(const vector<Match> & matches) {
	int maxNameLen = 0;
	for(auto & match : matches) {
		if(match.nameLen > maxNameLen) {
			maxNameLen = match.nameLen;
		}
	}
	maxNameLen += 2;

	for(auto & match : matches) {
		std:: cout << match.score << std::endl;
		std::cout << match.name;
		for(int i = 0; i < maxNameLen - match.nameLen; i++) {
			std::cout << " ";
		}
		std::cout << match.descr[0].str() << std::endl;
		if(match.descr.size() > 1) {
			for(int i = 0; i < maxNameLen; i++) {
				std::cout << " ";
			}
			std::cout << match.descr[1].str() << std::endl;
		}
	}
}

void find(vector<string> searchTerms) {

	vector<TokenizedPage> tpages = getAllPages();

	for(auto & tpage : tpages) {
		// stem all pages
		for(Line & line : tpage.descr) {
			for(Token & token : line) {
				stem(token.srch);
			}
		}	
		for(SearchableExample & example : tpage.examples) {
			for(Token & token : example.descr) {
				stem(token.srch);
			}
		}

		/* //remove all stop words
		for(auto & descrLine : tpage.descr) {
			removeStopWords(descrLine);
		}
		for(auto & example : tpage.examples) {
			removeStopWords(example.descr);
		}
		*/
	}

	// stem all search terms
	for(string & searchTerm : searchTerms) {
		stem(searchTerm);
	}

	string search_term = searchTerms[0];

	/*
	Match scoring system:
		If the page name matches exactly, the page get's 1000 points
		If the page name contains the search term it get's 500 points
		If a matched page name has subcommands, only the 3 highest scored are shown

		For each occurence in the first description line the page gets 100 points
		For each occurence in the first description line the page gets 50 points
		For each occurence in an example description, the page get's 50 points
		Only the 3 highest scored examples of a page are shown
	*/

	vector<Match> matches;

	for(auto & tpage : tpages) {
		Match match;
		int pos = tpage.name.find(search_term);
		if(pos != string::npos) {
			match.name = tpage.name;
			match.nameLen = tpage.name.length();
			match.name.insert(pos + search_term.length(), global::color::dfault);
			match.name.insert(pos, global::color::foundMatch);
			match.descr.push_back(tpage.descr[0]);
			if(tpage.name == search_term) {
				match.score += 1000;
			} else {
				match.score += 500;
			}
		}
		for(int i = 0; i < tpage.descr.size(); i++) {
			for(int j = 0; j < tpage.descr[i].size(); j++) {
				if(tpage.descr[i].at(j).srch == search_term) {
					if(match.name.empty()) {
						match.name = tpage.name;
						match.nameLen = tpage.name.length();
					}
					match.descr.clear();
					for(int k = 0; k < k + 1; k++) {
						match.descr.push_back(tpage.descr[k]);
					}
					match.descrMatchedAt.push_back({i, j});
					match.score += 50 * (2 - i);
				}
			}
		}
		if(!match.name.empty()) {
			matches.push_back(match);
		}
	}

	std::sort(matches.begin(), matches.end(), [](const Match & x, const Match & y) {return x.score > y.score;});
	printMatches(matches);
}

void removeStopWords(vector<string> & list) {

	static vector<string> stopWords = { // from https://gist.github.com/sebleier/554280
		"it", "its", "itself", "they", "them", "their", "theirs", "themselves", "what",
		"which", "who", "whom", "this", "that", "these", "those", "am", "is", "are", "was", "were", "be", "been", "being", "have", "has", "had",
		"having", "do", "does", "did", "doing", "a", "an", "the", "and", "but", "if", "or", "because", "as", "until", "while", "of", "at", "by",
		"for", "with", "about", "against", "between", "into", "through", "during", "before", "after", "above", "below", "to", "from", "up",
		"down", "in", "out", "on", "off", "over", "under", "again", "further", "then", "once", "here", "there", "when", "where", "why", "how",
		"all", "any", "both", "each", "few", "more", "most", "other", "some", "such", "no", "nor", "not", "only", "own", "same", "so", "than",
		"too", "very", "s", "t", "can", "will", "just", "don", "should", "now"
	};

	for(int i = list.size() - 1; i >= 0; i--) {
		for(const auto & stopWord : stopWords) {
			if(list[i] == stopWord) {
				list.erase(list.begin() + i);
			}
		}
	}
}

vector<string> tokenize(string s) {
	vector<string> tokens;
	std::stringstream tokenlist(s);
	while(tokenlist.good()) {
		string nextToken;
		std::getline(tokenlist, nextToken, ' ');
		tokens.push_back(nextToken);
	}
	return tokens;
}

/* 
Stemming algorithm
It can stem the 60 000 most popular english words in 0.5 seconds
More info: https://tartarus.org/martin/PorterStemmer/def.txt
There are some issues with "knightly" and "consolingly", but I couldn't figure out why.
*/
void stem(string & word) {
	int mval;
	
	/*
	In each step, only the longest match which could be applied, ignoring the condition, is even considered. The rest is ignored.
	For example, in step 1b, "feed" matches the first rule (because the condition m > 0 is ignored for now). And therefore we directly continue with 1c.
	*/

	// Step 1a
	if(word.ends_with("sses")) { // SSES -> SS
		word.erase(word.length() - 2, 2);
	} 
	else if(word.ends_with("ies")) { // IES -> I
		word.erase(word.length() - 2, 2);
	}
	else if(word.ends_with("s") && !word.ends_with("ss")) { // S -> NULL, but SS -> SS
		word.erase(word.length() - 1, 1);
	}

	// step 1b
	bool step1bAddition = false;
	if(word.ends_with("eed")) { // (m > 0) EED -> EE
		mval = getMval(word.substr(0, word.length() - 3));
		if(mval > 0) {
			word.erase(word.length() - 1, 1);
		}	
	}
	else if(word.ends_with("ed")) { // (*v*) ED -> NULL
		if(containsVowel(word.substr(0, word.length() - 2))) {
			word.erase(word.length() - 2, 2);
			step1bAddition = true;
		}
	}
	else if(word.ends_with("ing")) { // (*v*) ING -> NULL
		if(containsVowel(word.substr(0, word.length() - 3))) {	
			word.erase(word.length() - 3, 3);
			step1bAddition = true;
		}
	}

	// step 1b - part 2
	if(step1bAddition) {
		// AT -> ATE
		// BL -> BLE
		// IZ -> IZE
		if(word.ends_with("at") || word.ends_with("bl") || word.ends_with("iz")) {
			word.push_back('e');
		}
		// (*d and not (*L or *S or *Z)) -> single letter
		else if(endsWithDoubleConsonant(word)) {
			int l = word.length();
			if(!(word.at(l - 3) == 'l' || word.at(l - 3) == 's' || word.at(l - 3) == 'z')) {
				word.pop_back();
			}
		}
		else if(endsWithCVC(word)) { // (m = 1 and *o) -> E
			mval = getMval(word);
			if(mval == 1) {
				word.push_back('e');
			}
		}
	}

	// step 1c
	if(word.ends_with('y')) {
		if(containsVowel(word.substr(0, word.length() - 1))) {
			word.at(word.length() - 1) = 'i';
		}
	}

	// step 2
	if(word.ends_with("ational")) {
		replace(word, 0, 7, "ate");
	} else if(word.ends_with("tional")) {
		replace(word, 0, 6, "tion");
	} else if(word.ends_with("enci")) {
		replace(word, 0, 4, "ence");
	} else if(word.ends_with("anci")) {
		replace(word, 0, 4, "ance");
	} else if(word.ends_with("izer")) {
		replace(word, 0, 4, "ize");
	} else if(word.ends_with("abli")) {
		replace(word, 0, 4, "able");
	} else if(word.ends_with("alli")) {
		replace(word, 0, 4, "al");
	} else if(word.ends_with("entli")) {
		replace(word, 0, 5, "ent");
	} else if(word.ends_with("eli")) {
		replace(word, 0, 3, "e");
	} else if(word.ends_with("ousli")) {
		replace(word, 0, 5, "ous");
	} else if(word.ends_with("ization")) {
		replace(word, 0, 7, "ize");
	} else if(word.ends_with("ation")) {
		replace(word, 0, 5, "ate");
	} else if(word.ends_with("ator")) {
		replace(word, 0, 4, "ate");
	} else if(word.ends_with("alism")) {
		replace(word, 0, 5, "al");
	} else if(word.ends_with("iveness")) {
		replace(word, 0, 7, "ive");
	} else if(word.ends_with("fulness")) {
		replace(word, 0, 7, "ful");
	} else if(word.ends_with("ousness")) {
		replace(word, 0, 7, "ous");
	} else if(word.ends_with("aliti")) {
		replace(word, 0, 5, "al");
	} else if(word.ends_with("iviti")) {
		replace(word, 0, 5, "ive");
	} else if(word.ends_with("biliti")) {
		replace(word, 0, 6, "ble");
	}

	// step 3
	if(word.ends_with("icate")) {
		replace(word, 0, 5, "ic");
	} else if(word.ends_with("ative")) {
		replace(word, 0, 5, "");
	} else if(word.ends_with("alize")) {
		replace(word, 0, 5, "al");
	} else if(word.ends_with("iciti")) {
		replace(word, 0, 5, "ic");
	} else if(word.ends_with("ical")) {
		replace(word, 0, 4, "ic");
	} else if(word.ends_with("ful")) {
		replace(word, 0, 3, "");
	} else if(word.ends_with("ness")) {
		replace(word, 0, 4, "");
	}

	// step 4
	if(word.ends_with("al")) {
		replace(word, 1, 2, "");
	} else if(word.ends_with("ance")) {
		replace(word, 1, 4, "");
	} else if(word.ends_with("ence")) {
		replace(word, 1, 4, "");
	} else if(word.ends_with("er")) {
		replace(word, 1, 2, "");
	} else if(word.ends_with("ic")) {
		replace(word, 1, 2, "");
	} else if(word.ends_with("able")) {
		replace(word, 1, 4, "");
	} else if(word.ends_with("ible")) {
		replace(word, 1, 4, "");
	} else if(word.ends_with("ant")) {
		replace(word, 1, 3, "");
	} else if(word.ends_with("ement")) {
		replace(word, 1, 5, "");
	} else if(word.ends_with("ment")) {
		replace(word, 1, 4, "");
	} else if(word.ends_with("ent")) {
		replace(word, 1, 3, "");
	} else if(word.ends_with("ion")) {
		int l = word.length();
		if(l > 3) {
			if(word.at(l - 4) == 's' || word.at(l - 4) == 't') {
				replace(word, 1, 3, "");
			}
		}
	} else if(word.ends_with("ou")) {
		replace(word, 1, 2, "");
	} else if(word.ends_with("ism")) {
		replace(word, 1, 3, "");
	} else if(word.ends_with("ate")) {
		replace(word, 1, 3, "");
	} else if(word.ends_with("iti")) {
		replace(word, 1, 3, "");
	} else if(word.ends_with("ous")) {
		replace(word, 1, 3, "");
	} else if(word.ends_with("ive")) {
		replace(word, 1, 3, "");
	} else if(word.ends_with("ize")) {
		replace(word, 1, 3, "");
	}

	// step 5a
	if(word.ends_with('e')) {
		mval = getMval(word.substr(0, word.length() - 1));
		if(mval > 1) {
			word.pop_back();
		} else if (mval == 1 && !endsWithCVC(word.substr(0, word.length() - 1))) {
			word.pop_back();
		}
	}

	// step 5b
	if(word.ends_with("ll")) {
		mval = getMval(word.substr(0, word.length() - 1));
		if(mval > 1) {
			word.pop_back();
		}
	}
}

void replace(string & word, int minmval, int lbefore, string after) {
	int l = word.length();
	int mval = getMval(word.substr(0, l - lbefore));
	if(mval > minmval) {
		word.erase(l - lbefore, lbefore);
		word.append(after);
	}
}

bool endsWithCVC(string word) {
	int l = word.length();
	if(l < 3) {
		return false;
	}
	if(!isVowel(word.at(l - 3))) {
		if(isVowel(word.at(l - 2))) {
			if(!isVowel(word.at(l - 1))) {
				if(word.at(l - 1) != 'w' || word.at(l - 1) != 'x' || word.at(l - 1) != 'y') {
					return true;
				}
			}
		}
	}
	return false;
}

bool endsWithDoubleConsonant(string word) {
	int l = word.length();
	if(word.at(l - 1) == word.at(l - 2)) {
		if(isVowel(word.at(l - 1))) {
			return false;
		}
		return true;
	}
	return false;
}

bool isVowel(char c) {
	if(c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
		return true;
	}
	return false;
}

bool containsVowel(string word) {
	for(const auto & c : word) {
		// leaving out the part with `y`, because it's only a vowel if there's aleady another vowel
		if(c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
			return true;
		}
	}
	return false;
}

int getMval(string word) {
	bool vowel = false;
	int mval = 0;
	for(const char & c : word) {
		if(c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || (c == 'y' && vowel)) {
			vowel = true;
		} else {
			if(vowel) {
				mval++;
			}
			vowel = false;
		}
	}
	return mval;
}