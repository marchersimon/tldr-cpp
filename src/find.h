#pragma once

#include "global.h"
#include "cache.h"
#include "page.h"

class TokenizedPage {
    public:
        string name;
        vector<string> description;
        vector<vector<string>> exampleDescriptions;
        string getDescription();
};

void formatPage(Page & page);
vector<TokenizedPage> getAllPages();
void stem(string & word);
int getMval(string word);
bool containsVowel(string word);
bool isVowel(char c);
bool endsWithDoubleConsonant(string word);
bool endsWithCVC(string word);
void replace(string & word, int minmval, int lbefore, string after);
void removeStopWords(vector<string> & list);
vector<string> tokenize(string s);
void find(vector<string> search_terms);
