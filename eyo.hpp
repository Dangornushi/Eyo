#pragma once

#include <curses.h>
#include <unistd.h>

#include <fstream>
#include <iterator>
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <array>

#ifdef __linux
	#include <filesystem>
	#include <algorithm>
	namespace fs = std::filesystem;
#else
	namespace fs = std::__fs::filesystem;
#endif

#define NOMAL_M 0
#define INSERT_M 1
#define COMMAND_M 2
#define VISUAL_M 3

#define NOMAL 1
#define MACRO 2
#define TYPE 3
#define NUMBER 26
#define PARENTHESES 5
#define BRACKETS 6
#define VARIABLE 7
#define RESERVED 8
#define OP 9
#define COMMANDLINE 10
#define STATUS 11
#define SUBWIN 12
#define SP_RESERVED 13
#define COMMENT 14
#define FUNCTION 15
#define DARK_BLUE 16
#define RIGHT_GREANANDBLUE 17
#define NOMAL_COLOR 18
#define INSERT_COLOR 19
#define COMMAND_COLOR 20
#define NOMAL_MODE 21
#define INSERT_MODE 22
#define COMMAND_MODE 23
#define LINE 24
#define CONSECUTIVECOMMENT 25
#define NUM 26
#define LINENUM 24
#define VISUAL 0x8


using namespace std;

enum {
    kESC = 27,
    kBS = 8,
    kDEL = 127,
    kCtrlSpace = 0,
    kCtrlD = 4,
    kCtrlF = 6,
    kCtrlJ = 10,
    kEnter = 13,
    kCtrlN = 14,
    kCtrlP = 16,
    kCtrlQ = 17,
    kCtrlR = 18,
    kCtrlS = 19,
    kCtrlU = 21,
    kCtrlW = 23,
    VisualMode = 3,
};

typedef struct {
    string word;
    int type;
} Token;

extern void init();
extern void run();

extern bool split_token(string::iterator data, const char *word, int index);

extern void tokenPaint(int *nowToken, int *tokenCounter, const int len, const int attribute);
extern vector<Token> initPredictiveTransform();
extern void backChange();

extern void display();
extern void redraw();
extern void quit();
extern void save();

extern void insertMode();
extern void visualMode();
extern void del();
extern void finderCursor();
extern void globalInit();

extern int w, h;
extern bool gDone;
extern int windows;
extern int gIndex;
extern int gLines;
extern int nowMode;
extern int visualEnd;
extern int nowWindow;
extern int nowLineNum;
extern int nowLineBuf;
extern int gCol, gRow;
extern int gUndoIndex;
extern int visualStart;
extern int finderDrawBuf;
extern bool finderSwitch;
extern const char *gFileName;
extern string commandLineWord;
extern int LineStart, LineEnd;
extern int gPageStart, gPageEnd;
extern vector<string> finderData;
extern vector<char> gBuf, gUndoBuf;
