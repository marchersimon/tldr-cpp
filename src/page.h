#pragma once

#include <vector>
#include <iostream>
#include <sstream>

#include "global.h"

class Example {
    public:
        string description;
        string example;
};

class Page {
    private:
        int pos = 0;
        string getLine();
    public:
        string name;
        string description;
        string content;
        std::vector<Example> examples;

        Page(string content) {
            this->content = content;
            // get name stored in first line
            name = getLine();

            // ignore empty line
            getLine();

            while(1) {
                string line = getLine();
                if(line.empty()) {
                    break;
                }
                description.append(line + '\n');
            }

            while(1) {
                Example example;
                example.description = getLine();
                getLine();
                example.example = getLine();
                examples.push_back(example);
                if(getLine() == "EOF") {
                    break;
                }
            }
        }

        void print();
        void format();
};

string Page::getLine() {
    string line;
    int nextEOL = content.find('\n', pos);
    if(nextEOL == string::npos) {
        return "EOF";
    }
    line = content.substr(pos, nextEOL - pos);
    pos = nextEOL + 1;
    return line;
}

void Page::print() {
    std::cout << name << std::endl << std::endl;
    std::cout << description;
    for(auto example : examples) {
        std::cout << std::endl << example.description << std::endl << std::endl;
        std::cout << example.example << std::endl;
    }
}

void Page::format() {
    name.erase(0, 2);
    name.insert(0, global::nameColor);
    name.append(global::textColor);
}