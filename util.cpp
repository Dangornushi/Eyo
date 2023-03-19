#include "eyo.hpp"

bool nowLinux() {
    #ifdef __linux
        return true;
    #else
        return false;
    #endif
}

// util
bool find(const string s, const vector<Token> v) {
    for (auto autoV : v)
        if (s == autoV.word)
            return true;
    return false;
}


bool split_token(vector<char> v, int i, const char *word, int index) {
    char data = v.at(i);
    for (int i = 0; i < index; i++)
        if (data != word[i])
            return false;
    return true;
}

bool isChar(char data) {
    return (data >= 'a' && data <= 'z') || 
           (data >= 'A' && data <= 'Z') ||
           (data >= '0' && data <= '9') ||
           (data == '_')
               ? true
               : false;
}

void index(const string word, const vector<char> vec) {
    int v = gIndex;
    bool passd = false;

    for (;gIndex < vec.size() && !passd;)
        for (int i = 0; i<word.size()-1;i++, display())
            if (!(passd = ((vec[gIndex++] != word[i]) == false)))
                break;

    (vec.size() > gIndex) ? gIndex : gIndex = v;
}

bool isFunction(vector<char> v, int i) {
    char data; 
    for (;i < gBuf.size();i++) {
        data = v.at(i);
        if (data == '(')
            return true;
        if (data == ' ' || !isChar(data))
            return false;
    }
    return false;
}

