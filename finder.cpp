#include "eyo.hpp"
#include <algorithm>

namespace fs = std::__fs::filesystem;
const fs::directory_iterator end;
int fCol;
int fRow;
int dirFilesNum;
void finder();
void drawFinder();
vector<string> fileAndDirS;

void drawLinenumAndFinder(string *lineNumberString, int *c, const int AllLineLength) {
    color(COMMANDLINE);
    printw(" ");

    *lineNumberString = to_string(LineStart + (*c)++) + " ";
    for (; AllLineLength+finderDrawBuf > (*lineNumberString).size();
        (*lineNumberString).insert(0, " "));

    (*lineNumberString).insert(0, " ");
    color(LINE);
    printw("%s", (*lineNumberString).c_str());
}

void drawTildeAndFinder(string *lineNumberString, const int AllLineLength) {
    color(COMMANDLINE);
    printw(" ");

    color(LINE);
    printw("%s", (*lineNumberString).c_str());
}

void printDirectoryEntry(const fs::directory_entry &entry, int *maxNameLength) {
    string fileOrDir = entry.path().string();//.erase(0, 2);
    fileOrDir = fileOrDir.erase(0, 2);

    if (entry.is_directory())
        finderData.push_back("D " + fileOrDir);

    else if (entry.is_regular_file())
        finderData.push_back(fileOrDir = "F " + fileOrDir);

    fileAndDirS.push_back(fileOrDir);

    (fileOrDir.length() > *maxNameLength) ? *maxNameLength = fileOrDir.length() : 0;
    dirFilesNum++;
    
}

void finderQuit() {
    windows--;
    finderSwitch = false;
    display();
}

void drawInDir(const bool finderSwitch, const string lineNumberString, const int index) {

    if (!finderSwitch) {
        //nowLineBuf=2;
        return;
    }
    color(NOMAL);
}

void drawFinder() {
    int maxNameLength = 0;
    fCol = 0;
    fRow = 2;
    dirFilesNum = 0;

    finderData.clear();

    try {
        fs::directory_iterator it{"."};

        for (const fs::directory_entry &entry : it)
            printDirectoryEntry(entry, &maxNameLength);
    } catch (const fs::filesystem_error &e) {
        quit();
        printw("ディレクトリ内のファイルを参照する際に問題が発生しました\n");
    }

    finderSwitch = true;
    display();

    move(fRow, fCol);
    refresh();
}

void finder() {
    if (finderSwitch)
        finderQuit();
    else
        drawFinder();
}

