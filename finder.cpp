#include "eyo.hpp"
#include <algorithm>

#ifdef __linux
	#include <filesystem>
	namespace fs = std::filesystem;
#else
	namespace fs = std::_fs::filesystem;
#endif

const fs::directory_iterator end;
int fCol;
int fRow;
int dirFilesNum;
void finder();
void drawFinder();
vector<string> fileAndDirS;

void drawLinenumAndFinder(string *lineNumberString, int *c, const int AllLineLength) {
    attrset(COLOR_PAIR(COMMANDLINE));
    printw(" ");

    *lineNumberString = to_string(LineStart + (*c)++) + " ";
    for (; AllLineLength+finderDrawBuf > (*lineNumberString).size();
        (*lineNumberString).insert(0, " "));

    (*lineNumberString).insert(0, " ");
    attrset(COLOR_PAIR(LINE));
    printw("%s", (*lineNumberString).c_str());
}

void drawTildeAndFinder(string *lineNumberString, const int AllLineLength) {
    attrset(COLOR_PAIR(COMMANDLINE));
    printw(" ");

    attrset(COLOR_PAIR(LINE));
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
    attrset(COLOR_PAIR(NOMAL));
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

