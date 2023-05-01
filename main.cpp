// main.cpp   mushi's editor 2022.
// Github; Dangornushi-eyo

#include "finder.hpp"
#include "move.hpp"
#include "eyo.hpp"
#include "util.hpp"
#include "visualMode.hpp"

const fs::directory_iterator end;
const char *gFileName;

vector<char> gBuf, gUndoBuf;
vector<string> finderData;
vector<Token> predictive = initPredictiveTransform();
string commandLineWord;
string yankBuf;

bool gDone = false;
int gIndex;
int gPageStart;
int gPageEnd;
int gLines;
int gCol, gRow;
int gUndoIndex = 0;

int finderDrawBuf = 0;
int nowLineNum = 1;
int LineStart;
int LineEnd;
int w, h;

int moveDiff;
int nowMode;
int BACK;
int renderingLineNum;
int colorSet;
int nowLineBuf = 1;
int nowWindow;
int windows;
int visualStart;
int visualEnd;
bool classical;
bool finderSwitch = false;
bool terminalSwitch = false;

void globalInit() {
    nowMode = NOMAL_MODE;
    gIndex = 0;
    // offset of cursor pos
    gPageStart = 0;
    gPageEnd = 0;
    gLines = 0;
    nowLineBuf = 0;
    gBuf = {};

    nowLineNum = 1;
    LineStart = 0;
    LineEnd = 0;
    BACK = 0;
    renderingLineNum = 1;
    colorSet = 1;

    visualStart = 0;
    visualEnd = 0;
}


// TAG: Tokens
vector<Token> initPredictiveTransform() {
    vector<Token> vec{
        // all
        Token{"else ", RESERVED},
        Token{"if ", RESERVED},
        Token{"int ", TYPE},
        Token{"return ", RESERVED},
        Token{"for ", RESERVED},
        Token{"void ", TYPE},
        Token{"while ", RESERVED},
        Token{"class ", RESERVED},

        // C/C++
        Token{"break", RESERVED},
        Token{"char ", TYPE},
        Token{"const ", SP_RESERVED},
        Token{"case ", RESERVED},
        Token{"typedef ", SP_RESERVED},
        Token{"extern ", SP_RESERVED},
        Token{"struct ", RESERVED},
        Token{"enum ", RESERVED},
        Token{"map", TYPE},
        Token{"namespace ", RESERVED},
        Token{"vector", TYPE},
        Token{"bool ", TYPE},
        Token{"string ", TYPE},
        Token{"short ", TYPE},
        Token{"#define ", MACRO},
        Token{"#include ", MACRO},
        Token{"printf", RESERVED},
        Token{"public", SP_RESERVED},
        Token{"private", SP_RESERVED},
        Token{"uint32", TYPE},
        Token{"uint8", TYPE},
        Token{"using ", SP_RESERVED},
        Token{"/*", CONSECUTIVECOMMENT},
        Token{"*/", CONSECUTIVECOMMENT},
        Token{"//", COMMENT},

        // Python
        Token{"elif ", RESERVED},
        Token{"lamda ", RESERVED},
        Token{"def ", RESERVED},
        Token{"__init__ ", RESERVED},
        Token{"__main__ ", RESERVED},
        Token{"self", TYPE},

        // Rust
        Token{"fn ", RESERVED},
        Token{"->", OP},
        Token{"i8", TYPE},
        Token{"i32", TYPE},
        Token{"i64", TYPE},
        Token{"u8", TYPE},
        Token{"u32", TYPE},
        Token{"u64", TYPE},

        // Vim
        Token{"set", RESERVED},
        Token{"call", RESERVED},
        Token{"let", RESERVED},
        Token{"syntax", RESERVED},
        Token{"on", TYPE},
        Token{"off", TYPE},
        Token{"enable", TYPE},
        Token{"disable", TYPE},
    };
    return vec;
}

// TAG: predict
// 予測変換のサブウィンドウ
vector<Token> predictiveWin(const string word, const vector<Token> vec, const int index) {
    int i = 0;
    int ignoreBuf = 0;
    int type;
    bool ok = false;
    string data;
    vector<Token> newVec;

    if (vec.size() < index + 1) return vec;

    savetty();

    for (int j = 0; j < vec.size();
         data = vec[j].word, type = vec[j].type, j++) {
        attrset(COLOR_PAIR(COMMANDLINE));
        if (data[0] == word[0]) ok = true;
        if (data[0] != word[0]) ok = false;
        if (data[word.size() - 1] == word[word.size() - 1] && ok == true) {
            if (index + ignoreBuf == j) attrset(COLOR_PAIR(STATUS));

            mvaddstr(gRow + ++i, gCol + nowLineBuf, data.c_str());
            newVec.push_back(Token{data, type});
        } else
            ignoreBuf++;
    }
    resetty();

    return newVec;
}

// no move
void save() {
    ofstream ofs(gFileName, ios::binary);
    ostream_iterator<char> output_iterator(ofs);
    copy(gBuf.begin(), gBuf.end(), output_iterator);
    commandLineWord = "saved";
    redraw();
}

void renderingNowLine() {
    string number = " " + to_string(nowLineNum) + " : " + to_string(gCol) + " ";

    savetty();

    attrset(COLOR_PAIR(SUBWIN));
    mvaddstr(gRow + 1, gCol + nowLineBuf + 1, number.c_str());
    move(gRow, gCol + nowLineBuf);
    refresh();
    getch();

    resetty();
}

void wordJump() {
    string jumpWordBuf;
    int jumpWordIndex = 0;

    for (int ch;; display(), savetty()) {
        if ((ch = getch()) == kESC)
            break;
            
        else if (jumpWordIndex > 0 && (ch == kBS || ch == kDEL)) {
            jumpWordBuf.erase(jumpWordBuf.begin() + (--jumpWordIndex));
            nowLineNum--;
        }

        else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                 (ch >= '0' && ch <= '9') || (ch == '_'))
            jumpWordBuf.insert(jumpWordBuf.begin() + jumpWordIndex++,
                               ch == '\r' ? '\n' : ch);

        else {
            jumpWordBuf.insert(jumpWordBuf.begin() + jumpWordIndex++, ch);
            if (ch == '\n') break;
        }
        commandLineWord = jumpWordBuf;
    }
    index(jumpWordBuf, gBuf);
    redraw();
}

void newOpen() {
/*
    clear();
    refresh();
    resetty();
    *///
    
    globalInit();
    savetty();
    run();
}

void renderingFinder() {
    string gFileNameBuf;
    int winW = 0;
    int winH = (h - 1);
    savetty();

    move(winH, 0);
    mvaddstr(winH, winW, "[ open filename ]");
    move(winH, 0);

    attrset(COLOR_PAIR(COMMANDLINE));

    for (int ch;;) {
        if ((ch = getch()) == kESC)
            return;

        else if ((ch == kBS) || (ch == kDEL))
            gFileNameBuf.erase(gFileNameBuf.end() - 1);

        else if (ch == '\n')
            break;
        else
            gFileNameBuf.insert(gFileNameBuf.end(), ch);

        clrtoeol();
        mvaddstr(winH, winW, gFileNameBuf.c_str());
    }

    gFileName = gFileNameBuf.c_str();
    newOpen();
}

bool input(int *scrollBase, vector<string> lsData, int commandRow) {
    char ch;
    string fileNum = "";
    while (1) {
        switch (ch = getch()) {
            case 'k': {
                (*scrollBase > 0 ) ? (*scrollBase)-- : 0;
                return false;
            }
            case 'j': {
                if (*scrollBase < lsData.size()-1) (*scrollBase)++;
                return false;
            }
            case '\n': {
                fileNum = to_string(*scrollBase+1);
                gFileName = lsData.at(stoi(fileNum) - 1)
                                .erase(0, fileNum.length() + 1)
                                .c_str();

                LineStart = 0;
                LineEnd = 0;
                nowLineNum = 1;
                nowLineBuf = 1;
                gLines = 0;
                curs_set(1);
                run();
                exit(0);
                return true;
            }
            default:
                finderDrawBuf = 0;
                finderSwitch = false;
                return true;
        }
        refresh();
    }
}

void commandLineLs() {

    if (!finderSwitch) {
		finderDrawBuf = 0;
		return;
	}

    vector<string> lsData;
    curs_set(0);
    int fileNameWidth = 0;
    
    try {
        int num = 1;
        fs::directory_iterator it{"."};
        for (const fs::directory_entry &entry : it) {
            string fileOrDir = entry.path().string();  //.erase(0, 2);
            fileOrDir = to_string(num++) + " " + fileOrDir.erase(0, 2);
            lsData.push_back(fileOrDir);
            (fileOrDir.length() > fileNameWidth) ? fileNameWidth = fileOrDir.length()+2 : 0;
        }
    } catch (const fs::filesystem_error &e) {
        quit();
        printw("ディレクトリ内のファイルを参照する際に問題が発生しました\n");
        return;
    }
    reverse(lsData.end(), lsData.begin());
    char ch = 'a';
    int scrollBase = 0;
    int commandRow = h-1;

    finderDrawBuf = fileNameWidth;
    display();	
    
    for (;;) {
        int i = 0;
        int j = 0;		
        
        attrset(COLOR_PAIR(NOMAL));
        int tw=0;
        mvaddstr(i, tw++, "  ");
        for (tw++;tw<fileNameWidth;mvaddstr(i, tw++, "_"));
        mvaddstr(i++, tw++, " ");
       
        // フォルダ内のファイル名を描写
        for (;j < lsData.size(); j++) {
            int k = lsData[j].length() + 2; 
            if (i > h-3) break;
           	
            // 左側の縦枠を描写
            attrset(COLOR_PAIR(NOMAL));
            mvaddstr(i, 1, "|");
            
            if (j == scrollBase)
            	attrset(COLOR_PAIR(NOMAL_MODE));
            // ファイルあるいはディレクトリないの何かの名前を描写
            mvaddstr(i, 2, lsData[j].c_str());
            for (; k < fileNameWidth;mvaddstr(i, k++, " "));
                      	
            // 右側の縦枠を描写
            attrset(COLOR_PAIR(NOMAL));
            mvaddstr(i, k, "| ");
           
            // 高さを一つ下げる
            i++;
       }
        
       // 余分な空白を描写
       for (;i < h-2; i++) {
           attrset(COLOR_PAIR(NOMAL));
           int tw=0;
           mvaddstr(i, tw++, " |");
           tw++;
           for (;tw<fileNameWidth;mvaddstr(i, tw++, " "));
           mvaddstr(i, tw, "| ");            
       }
       
       tw=0;
       attrset(COLOR_PAIR(NOMAL));
       mvaddstr(i, tw++, " "); 
       tw++;
       for (;tw<fileNameWidth;mvaddstr(i, tw++, "-"));
       
       refresh();
       if (input(&scrollBase, lsData, commandRow)) break;
    }
    curs_set(1);
    return;
}

string inputBox() {

	string runCommandBuf;
            
    for (int ch;; display(), savetty()) {
	    if ((ch = getch()) == kESC)
            break;
        else if (runCommandBuf.length() > 0 && (ch == kBS || ch == kDEL)) {
        	runCommandBuf.pop_back();
        }
            	
		else {
            runCommandBuf += ch;
        	if (ch == '\n') break;
        }
        commandLineWord = runCommandBuf;
    }
    
    return runCommandBuf;
}

// Command input
void commandMode() {
    nowMode = COMMAND_MODE;
    string before_commandLineWord = commandLineWord;
    commandLineWord = "> ";
    redraw();

    for (int ch = getch();;) {
        commandLineWord = ch;
        redraw();

        if (kESC == ch) break;

        if ('w' == ch) {
            save();
            break;
        }

        if ('q' == ch) {
            quit();
            break;
        }

        if ('o' == ch) {
            renderingFinder();
            break;
        }

        if ('r' == ch) {
            resetty();
            LineStart = 0;
            run();
            break;
        }

        if ('l' == ch) {
            renderingNowLine();
            break;
        }
        
        if ('g' == ch) {
            int gotoLine;           
            string runCommandBuf;
            
    		gotoLine = stoi(inputBox());
    		
    		int i = 0;
    		int gMinN = gotoLine - nowLineNum; 		
    		
	        if (gMinN > 0) {
				gIndex = 0;
        		for (int lineCounter=1;gIndex<gBuf.size();gIndex++) {
        			if (gBuf[gIndex] == '\n') {	
						//display();
						if (lineCounter<gotoLine)
							lineCounter++;	
						else 
        					break;	
	        		}
	        	}
        	} else {
        		for (int lineCounter=1;gIndex>-(nowLineNum-gMinN);gIndex--) {
            		/*
            		if (gBuf[gIndex] == '\n')
        				display();	
				*///
	        	}
	        	
	        }
	        
        	break;
        }
        
        if ('$' == ch) {
    		system(inputBox().erase(0, 1).c_str());
        	break;
        }
        break;
    }

    // commandLineWord = before_commandLineWord;
    nowMode = NOMAL_MODE;
    redraw();
}

void addCounter(char c, char s, int spaceNum) {
    for (int i = 0; c == '{' && spaceNum > i; i++)
        gBuf.insert(gBuf.begin() + gIndex++, s);
    gBuf.insert(gBuf.begin() + gIndex++, s);
}

void addCounter(char c, char s) {
    if (c == '{') gBuf.insert(gBuf.begin() + gIndex++, s);
    gBuf.insert(gBuf.begin() + gIndex++, s);
}

int getEndIndex(int i) {
    while (gBuf[i] != '\n') i++;
    return i;
}

void whiteSpaceCompletion() {
    int spaceCounter = 0;
    int tabCounter = 0;

    for (int i = lineTop(gIndex - 1);; i++) {
        if (gBuf[i] == ' ')
            addCounter(gBuf[gIndex - 2], ' ', 4);
        else if (gBuf[i] == '\t')
            addCounter(gBuf[gIndex - 2], '\t');
        else
            break;
    }
}

void newLine() {
    lineEnd();
    gBuf.insert(gBuf.begin() + gIndex++, '\n');
    whiteSpaceCompletion();
    gRow++;
    redraw();
    insertMode();
}

void lineDel() {
    /*
    for (; gBuf[gIndex] != '\n'; gIndex++) {
        ++moveDiff;
        yankBuf.push_back(gBuf[gIndex]);
    }
    for (int i = 0; i < gCol; i++)
        yankBuf.push_back(gBuf[gIndex++]);
    */
}

// insertMode
void insertMode() {
    nowMode = INSERT_MODE;
    commandLineWord = "! insert !";
    redraw();
    int viewIndex = -1;
    int tmpBuf, tmpIndex = gIndex;
    bool tabStart = false;
    string nowInputWord;
    vector<Token> newPredictive;

    gUndoBuf = gBuf;
    gUndoIndex = gIndex;

    for (int ch;;) {
        int tmpLineBuf = nowLineBuf;
        redraw();
        nowLineBuf = tmpLineBuf;
        savetty();
        if (!classical) {
            newPredictive.clear();
            newPredictive = predictiveWin(nowInputWord, predictive, viewIndex);
        }

        move(gRow, gCol + nowLineBuf);

        if ((ch = getch()) == kESC) {
            viewIndex = -1;
            break;
        }

        else if (gIndex > 0 && (ch == kBS || ch == kDEL)) {
            (gBuf[--gIndex] == '\n') ? gLines-- : 0;

            if ((gBuf[gIndex] == '(' && gBuf[gIndex + 1] == ')') ||
                (gBuf[gIndex] == '{' && gBuf[gIndex + 1] == '}') ||
                (gBuf[gIndex] == '[' && gBuf[gIndex + 1] == ']') ||
                (gBuf[gIndex] == '"' && gBuf[gIndex + 1] == '"') ||
                (gBuf[gIndex] == '\'' && gBuf[gIndex + 1] == '\'')) {
                gBuf.erase(gBuf.begin() + (gIndex + 1));
            }
            
            else if ((gBuf[gIndex-1] == '(' && gBuf[gIndex] == ')') ||
                (gBuf[gIndex-1] == '{' && gBuf[gIndex] == '}') ||
                (gBuf[gIndex-1] == '[' && gBuf[gIndex] == ']') ||
                (gBuf[gIndex-1] == '"' && gBuf[gIndex] == '"') ||
                (gBuf[gIndex-1] == '\'' && gBuf[gIndex] == '\'')) {
                    gIndex+=2;                
                    continue;
            }


            gBuf.erase(gBuf.begin() + (gIndex));
            if (nowInputWord.size() > 0)
                nowInputWord.resize(nowInputWord.size() - 1);
            nowLineNum--;
            viewIndex = -1;
        }

        else if (!classical && ch == kCtrlN) {
            if (viewIndex == -1)
                // CtrlNの一番上
                for (int i = gIndex; i > tmpIndex; i--, gIndex--)
                    gBuf.erase(gBuf.begin() + gIndex - 1);

            else
                // 一番上以外
                for (int i = 0; i < newPredictive[viewIndex].word.size(); i++)
                    gBuf.erase(gBuf.begin() + (gIndex--) - 1);

            (viewIndex < newPredictive.size()) ? viewIndex++ : viewIndex = 0;

            for (int i = 0; i < newPredictive[viewIndex].word.size(); i++)
                gBuf.insert(gBuf.begin() + gIndex++,
                            newPredictive[viewIndex].word[i] == '\r'
                                ? '\n'
                                : newPredictive[viewIndex].word[i]);

            newPredictive.clear();
        }

        else if (!classical && ch == kCtrlP) {
            if (newPredictive.size() < viewIndex) continue;

            for (auto ch : newPredictive[viewIndex].word)
                gBuf.erase(gBuf.begin() + (--gIndex));

            (viewIndex > -1) ? viewIndex--
                             : viewIndex = newPredictive.size() - 1;

            for (auto ch : newPredictive[viewIndex].word)
                gBuf.insert(gBuf.begin() + gIndex++, ch == '\r' ? '\n' : ch);

            newPredictive.clear();
        }

        else if (ch == '{' || ch == '[' || ch == '(') {
            gBuf.insert(gBuf.begin() + gIndex++, ch);
            gBuf.insert(gBuf.begin() + gIndex, (ch == '(') ? ch + 1 : ch + 2);
        }

        else if (ch == '\'' || ch == '"') {
            gBuf.insert(gBuf.begin() + gIndex++, ch);
            gBuf.insert(gBuf.begin() + gIndex, ch);
        }

        else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                 (ch >= '0' && ch <= '9') || (ch == '_')) {
            gBuf.insert(gBuf.begin() + gIndex++, ch);
            (!classical) ? nowInputWord.push_back(ch) : void();
        }

        else {
            gBuf.insert(gBuf.begin() + gIndex++, ch);

            if (ch == '\n') {
                whiteSpaceCompletion();

                nowLineNum++;
                gLines++;
            }
            tmpIndex = gIndex;

            viewIndex = -1;

            if (!classical) {
                predictive.push_back(Token{nowInputWord, NOMAL});
                nowInputWord.clear();
            }
        }
    }
    resetty();
    nowMode = NOMAL_MODE;
    commandLineWord = " NOMAL ";
}

//   world
//   world 
//   Hello,

void paste() {
    (moveDiff < 0) ? reverse(yankBuf.begin(), yankBuf.end()) : void();
    for (auto ch : yankBuf)
        gBuf.insert(gBuf.begin() + ++gIndex, ch == '\r' ? '\n' : ch);
    (moveDiff < 0) ? reverse(yankBuf.begin(), yankBuf.end()) : void();
}

void addInsert() {
    gIndex++;
    redraw();
    insertMode();
}

void undo() {
    gBuf.clear();
    copy(gUndoBuf.begin(), gUndoBuf.end(), back_inserter(gBuf));
    gBuf = gUndoBuf;
    gIndex = gUndoIndex;
    redraw();
}

void windowChange() {
    (windows > nowWindow && windows > 1) ? nowWindow++ : nowWindow = 0;
}

int drawLine(int y, int rimit) {
	mvaddstr(y, 0, " ");
	for (int i=1;i<rimit-1;i++) {
		mvaddstr(y, i, "-");
	}
	mvaddstr(y, rimit, " ");
	return y++;
}

void printlnInTerminal(string word, int *x, int *y) {
	mvaddstr(*y, *x, word.c_str());
	
	for (*x += word.length();*x<w-1;(*x)++) {
		mvaddstr(*y, *x, " ");
	}
	
	mvaddstr(*y, *x, "\n");
}

bool ExecCmd(string cmd, string& stdOut, int& exitCode) {
	std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), [&](FILE* p) {exitCode = pclose(p); });
	if (!pipe) {
		return false;
	}
	std::array<char, 256> buf;
	while (!feof(pipe.get())) {
		if (fgets(buf.data(), buf.size(), pipe.get()) != nullptr) {
			stdOut += buf.data();
		}
	}
	return true;
}

string runCommand(string command, int* index) {
	string ret = "";
	string stdOut = "";
	int exitCode = 0;
				
		
	if (ExecCmd(command, stdOut, exitCode)) {
		if (stdOut == "")
			stdOut = "code: 0\n";
		for (int i=0;i<stdOut.size();i++) {
			if (stdOut.at(i) == '\n')
				(*index)++;
			else
				ret += stdOut.at(i);
		}
		return ret;
	}
	exit(1);
}
                                                                            
void terminal(string startCommand) {

    if (!terminalSwitch) {
		return;
	}

	int terminalHeight = 20;
	int terminalWidth = w;
	int terminalInputIndex = 0;
	vector<string> terminalPutBuf (terminalHeight, "");	

	terminalPutBuf.at(0) = runCommand(startCommand, &terminalInputIndex);
	
	for (;;) {	
		int startHeight = h - terminalHeight;	
		
		display();
		
		startHeight = drawLine(startHeight, terminalWidth);	
		startHeight++;	
		
		for (int terminalPutBufIndex=0;startHeight < h-2;) {	
			int x = 0;
			mvaddstr(startHeight, x++, "|");
			printlnInTerminal(terminalPutBuf.at(terminalPutBufIndex++), &x, &startHeight);
			mvaddstr(startHeight++, x, "|");
		}	
		
		startHeight = drawLine(startHeight, terminalWidth);
	
		refresh();	
		
		int ch = getch();
		switch (ch) {
			case kESC: {
				terminalSwitch = false;
				return;
			}
			case 127: {
				terminalPutBuf.at(terminalInputIndex).pop_back();
				
				break;
			}
			case '\n': {
				// 適当なコマンドを用意する
				string resultStr = runCommand(terminalPutBuf.at(terminalInputIndex), &terminalInputIndex);
				
				terminalPutBuf.at(terminalInputIndex++) = resultStr;
				
				break;
			}
			default: {
				terminalPutBuf.at(terminalInputIndex).push_back(ch);
				break;
			}
		}
	}
}

void finderOn() {
	finderDrawBuf = 0;
    if (finderSwitch == true) {
        finderSwitch = false;
    }
    else {
        finderSwitch = true;
    }
}

void terminalOn() {
	if (terminalSwitch)
		terminalSwitch = false;
	else
		terminalSwitch = true;
} 

unordered_map<char, void (*)()> gAction = {
    {'h', left},
    {'l', right},
    {'k', up},
    {'j', down},
    {'b', wordLeft},
    {'w', wordRight},
    {kCtrlD, pageDown},
    {kCtrlU, pageUp},
    {'0', lineBegin},
    {'$', lineEnd},
    {'T', top},
    {'g', top},
    {'G', bottom},
    {'i', insertMode},
    {'x', del},
    {kCtrlQ, quit},
    {kCtrlR, redraw},
    {kCtrlS, save},
    {'o', newLine},
    //{kCtrlW, finderCursor},
    {kCtrlF, finderOn},
    {'H', gotoUp},
    {'L', gotoDown},
    {'f', commandMode},
    {':', commandMode},
    {' ', commandMode},
    {'d', del},
    {'c', lineBegin},
    {'w', oneWordMove},
    {'b', oneWordBack},
    {'/', wordJump},
    {'v', visualMode},
    {'p', paste},
    {'a', addInsert},
    {'u', undo},
    {kCtrlJ, terminalOn},
};

void run() {
    ifstream ifs(gFileName, ios::binary);
    gIndex = 0;
    gBuf.assign(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>());
    gUndoBuf = gBuf;
    gUndoIndex = gIndex;

    nowMode = NOMAL_MODE;
    nowWindow = 0;
    windows = 1;
    commandLineWord = " NOMAL ";
    finderData = {""};
    finderSwitch = false;
    gPageStart = gPageEnd = 0;

    // split token
    if (!classical) {
        // 色設
        start_color();

        if (can_change_color() == true && has_colors() == true) backChange();

        string nt;

        for (auto data : gBuf) {
            
            if (isChar(data)) {
                nt.push_back(data);
                continue;
            }
            
            else if (!find(nt, predictive)) {
                predictive.push_back(Token{nt, NOMAL});
                predictive.push_back(Token{"", 0});
                nt.clear();
            }
            else{
                nt.clear();}
            
            if (data == '\n')
                gLines++;
       }
    }

    // start
    while (!gDone) {
        terminal("echo hello");
        commandLineLs();
        display();
        char ch = getch();
        if (gAction.count(ch) > 0) gAction[ch]();

        if (nowWindow == 1) finder();
    }
    endwin();
}

void init() {
    // init
    initscr();
    setlocale(LC_ALL, "");
    set_tabsize(4);
    raw();
    noecho();
    idlok(stdscr, true);  // init screen.
    getmaxyx(stdscr, h, w);

    classical = false;

    run();
}

int main(int argc, char **argv) {
    if (argc < 2) return 2;

    // file
    gFileName = argv[1];
    init();

    return 0;
}

