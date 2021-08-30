#pragma once

#include "../global.h"

void stem(string & word);
int getMval(string word);
bool containsVowel(string word);
bool isVowel(char c);
bool endsWithDoubleConsonant(string word);
bool endsWithCVC(string word);
void replace(string & word, int minmval, int lbefore, string after);