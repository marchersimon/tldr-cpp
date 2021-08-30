#include "stem.h"


/* 
Stemming algorithm
It can stem the 60 000 most popular english words in 0.5 seconds
More info: https://tartarus.org/martin/PorterStemmer/def.txt
*/
void stem(string & word) {
	
	// For short words this doesn't work very well. For example `ls` will be shortened to `l`.
	if(word.length() < 3) {
		return; 
	}
	
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