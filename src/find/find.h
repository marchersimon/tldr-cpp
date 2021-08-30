#pragma once

#include <set>
#include <numeric>
#include <math.h>
#include "../global.h"
#include "../cache.h"
#include "../page.h"
#include "stem.h"

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
        Line name;
        string platform;
        vector<Line> descr;
        vector<SearchableExample> examples;
        bool hasMatch = false;
        vector<double> tfidf;
        double termFrequency;
        double score;
};

class Match {
    public:
        bool matched = false;
        int score = 0;
        string name;
        int nameLen = 0;
        vector<Line> descr;
        vector<vector<int>> descrMatchedAt; // {line, word}
};

void formatPage(TokenizedPage & page);
vector<TokenizedPage> getAllPages();
void removeStopWords(Line & line);
vector<string> tokenize(string s);
vector<string> tokenizeBySpace(string s);
void find(vector<string> search_terms);
string join(vector<string> vect);
void printMatches(const vector<Match> & matches);
void highlightMatches(vector<Match> & matches);
