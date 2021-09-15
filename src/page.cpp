#include "page.h"

Page::Page(string content) {
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
        example.command = getLine();
        examples.push_back(example);
        if(getLine() == "EOF") {
            break;
        }
    }
}

Page::Page() {}

string Page::getLine() {
    string line;
    std::size_t nextEOL = content.find('\n', pos);
    if(nextEOL == string::npos) {
        return "EOF";
    }
    line = content.substr(pos, nextEOL - pos);
    pos = nextEOL + 1;
    return line;
}

void Page::print() {
    string indent = "  ";
    std::cout << '\n';
    if(!global::opts::platform.empty() && platform != global::opts::platform && global::opts::verbose) {
        std::cout << indent << "Displaying page from platform " << platform << "." << std::endl;
    }
    
    std::cout << indent << name << std::endl << std::endl;
    
    size_t pos = description.find('\n');
    while(pos != string::npos) {
        description.replace(pos, 1, "\n  ");
        pos = description.find('\n', pos + 1);
    }
    std::cout << indent << description;
    for(auto example : examples) {
        std::cout << std::endl << indent << example.description << std::endl << std::endl;
        std::cout << "    " << example.command << std::endl;
    }
}

int Page::getAgvDescrLen(string descr) {
    int numberOfLines = std::count(descr.begin(), descr.end(), '\n') + 1;
    int totalLen = descr.length();
    int len = totalLen / numberOfLines;
    return len;
}

void Page::format() {
    // format the title
    name.erase(0, 2);
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    name.insert(0, global::color::title);
    name.append(global::color::dfault);
    // center the title
    int avgDescrLen = getAgvDescrLen(description);
    name.insert(0, string(avgDescrLen - name.length() / 2, ' '));
    // Format the command description
    // Remove the "> " at the start of the line
    std::regex reg1("(\\> )");
    std::smatch matches;
    while(std::regex_search(description, matches, reg1)) {
        description.erase(matches.position(), 2);
    }
    // Remove the brackets of the more information link
    std::regex reg2("\\<.*\\>\\.");
    if(std::regex_search(description, matches, reg2)) {
        description.erase(matches.position() + matches.length() - 2, 1);
        description.erase(matches.position(), 1);
    }
    formatBackticks(description);

    // Colorize commands and token syntax
    for (auto & example : examples) {
        formatBackticks(example.description);
        example.command.replace(0, 1, global::color::command);
        example.command.replace(example.command.length() - 1, 1, global::color::dfault);
        formatTokenSyntax(example.command);
    }

    // replace example bullet points with ▸
    for(Example & example : examples) {
        example.description.replace(0, 1, "▸");
    }
}

void Page::formatBackticks(string & str) {
    std::regex reg("(\\`([^\\`].*?)\\`)");
    std::smatch matches;
    while(std::regex_search(str, matches, reg)) {
        str.replace(matches.position() + matches.length() - 1, 1, global::color::dfault);
        str.replace(matches.position(), 1, global::color::backtick);
    }
}

void Page::formatTokenSyntax(string & str) {
    int tokenCount = 0; // needed to detect nested "{{ }}"
    bool hasNestedTokens = false;
    std::regex reg("(\\{{2}|\\}{2}(?!\\}))");
    std::smatch matches;
    while(std::regex_search(str, matches, reg)) {
        if(matches.str() == "{{" && tokenCount == 0) {
                str.replace(matches.position(), 2, global::color::token);
                tokenCount++;
        } else if(matches.str() == "}}" && tokenCount == 1) {
            str.replace(matches.position(), 2, global::color::command);
            tokenCount--;
        } else if(matches.str() == "{{") {
            // insert 0x07 between braces so they don't get matched again
            str.insert(matches.position() + 1, "\7");
            tokenCount++;
            hasNestedTokens = true;
        } else if(matches.str() == "}}") {
            str.insert(matches.position() + 1, "\7");
            tokenCount--;
            hasNestedTokens = true;
        }
        if(tokenCount < 0) {
            tokenCount = 0;
        }
    }
    
    if(hasNestedTokens) { // remove all the 0x07 again
        std::size_t pos = str.find("\7");
        while(pos != string::npos) {
            str.erase(pos, 1);
            pos = str.find("\7");
        }
    }
}