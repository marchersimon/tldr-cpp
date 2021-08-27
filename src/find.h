#pragma once

#include "global.h"
#include "cache.h"
#include "page.h"

class Token {
    public:
        string orig; // origial
        string srch; // search
        Token(string origWord);
        Token(){};
};

class Line {
    public:
        vector<Token> line;
        Line(){};
        Line(vector<string> origLine);
        Token& at(int n);
        void remove(int n);
        vector<Token>::iterator begin();
        vector<Token>::iterator end();
        int size();
        string str() const;
};

class SearchableExample {
    public:
        Line descr;
        string command;
};

class TokenizedPage {
    public:
        string name;
        vector<Line> descr;
        vector<SearchableExample> examples;
        bool hasMatch = false;
};

class Match {
    public:
        int score = 0;
        string name;
        int nameLen = 0;
        vector<Line> descr;
        vector<vector<int>> descrMatchedAt; // {line, word}
};

void formatPage(TokenizedPage & page);
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
string join(vector<string> vect);
void printMatches(const vector<Match> & matches);
void highlightMatches(vector<Match> & matches);
