#pragma once

#include <iomanip>
#include <numeric>
#include <math.h>
#include "../global.h"
#include "../cache.h"
#include "../page.h"
#include "stem.h"

#define searchDescr (!global::opts::findOverrideDefaults || global::opts::description)
#define searchExamples (!global::opts::findOverrideDefaults || global::opts::examples)
#define searchName (!global::opts::findOverrideDefaults || global::opts::name)
#define searchNameOnly (global::opts::findOverrideDefaults && global::opts::name && !global::opts::description && !global::opts::examples)

class Token {
    public:
        string orig; // origial
        string srch; // search
        void makeSearchable();
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

class TExample {
    public:
        Line descr;
        string command;
};

class TPage {
    public:
        Line name;
        string platform;
        int namelen;
        vector<Line> descr;
        vector<TExample> examples;
        bool hasMatch = false;
        vector<double> tfidf;
        double termFrequency;
        double score;
        vector<bool> matchedDescriptionLines; // will store if each line has a match or not
        vector<bool> matchedExamples;
};

void formatPage(Page & page);
vector<TPage> getAllPages();
bool isStopWord(string word);
vector<string> tokenize(string s);
vector<string> tokenizeBySpace(string s);
void find(vector<string> search_terms);
string join(vector<string> vect);
void printMatches(const vector<TPage> & matches);
void highlightMatches(vector<TPage> & matches, vector<string> searchTerms);
