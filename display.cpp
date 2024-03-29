#include "eyo.hpp"
#include "move.hpp"
#include "util.hpp"
#include "finder.hpp"

void display() {

    int c = 1;

    if (gIndex < gPageStart) {
        // 上に移動
        if (gRow >= 0 && LineStart > 0) {
            LineStart--;
        }
        gPageStart = lineTop(gIndex);
    }

    else if(gPageEnd<=gIndex && gIndex < gBuf.size() && gRow >= h-1)  {
        ( gLines > h ) ? LineStart++ : 0;
        gPageStart=nextLineTop(gIndex);
        int n = LINES;
        for(int i=1; i<n; i++) { gPageStart=lineTop(gPageStart-1); }
    }

    move(0, 0);

    int x = 0;
    int y = 1;
    int tokenCounter = 0;
    int nowToken = 0;
    int tmpLineBuf = nowLineBuf;
    int AllLineLength = to_string(gLines).size()+1;
    string lineNumberString;
    bool nowComment = false;
    bool nowString = false;
    bool nowConsecutiveComment = false;
    
    
	// ツールバー
	string filename = gFileName;
    string cursorRow = " " + filename + " ";
    int consoleRow = 0; // ツールバーがある高さ

    attrset(COLOR_PAIR(nowMode));
    mvaddstr(consoleRow, 0, commandLineWord.c_str());
    attrset(COLOR_PAIR(COMMANDLINE));

    for (auto j=commandLineWord.size(); j < COLS - cursorRow.size();)
        mvaddstr(consoleRow, j++, " ");

    attrset(COLOR_PAIR(NOMAL));
    mvaddstr(consoleRow, COLS - cursorRow.size(), cursorRow.c_str());
    // 
    
    gPageEnd = gPageStart;

    drawLinenumAndFinder(&lineNumberString, &c, AllLineLength);

    nowLineBuf = lineNumberString.size() + 1;

    for (auto p = gBuf.begin() + gPageEnd;; gPageEnd++, p++) {

        if (gIndex == gPageEnd) {
            // update cursor pos.
            gRow = y;
            gCol = x;
        } 

        if (y == h || gBuf.size() <= gPageEnd) {
            break;
        }

        if (nowMode == VISUAL_M &&
            ((gPageEnd >= visualStart && gPageEnd < visualEnd) ||
             (gPageEnd < visualStart && gPageEnd >= visualEnd))) {

            attrset(COLOR_PAIR(NOMAL_MODE));

            (*p == '\t') ? printw("    ") : addch(*p);
            x += *p == '\t' ? 4 - (x & 3) : 1;
            if (LineStart+1 + c > gLines + 1) {
                y++;
                break;
            }

            if (*p == '\n' || COLS <= x) {
                drawLinenumAndFinder(&lineNumberString, &c, AllLineLength);
                x = 0;
            }
            continue;
        }
         
        if (gLines < LineStart+c-2) {
            nowComment = false;
            LineStart--;
        }
 
        if (*p == '\n' || COLS <= x) {
            printw("\n");
            drawInDir(finderSwitch, lineNumberString, ++y);
            drawLinenumAndFinder(&lineNumberString, &c, AllLineLength);
            x = 0;
            nowComment = false;
            attrset(COLOR_PAIR(NOMAL));
        }
        //if (*p != '\r') {
        else {
            // if the colour options was set
            switch (*p) {
	        	case '"': {
                    nowString = !nowString;
		            tokenPaint(&nowToken, &tokenCounter, 1, SP_RESERVED);
	        	    break;
	        	}
                case '<':
                case '>':
                case '+':
                case '-':
                case '*':
                case '=':
                case '&':
                case '$':
                case '%':
                    attrset(COLOR_PAIR(OP));
                    break;

                case '(':
                case ')':
                    attrset(COLOR_PAIR(PARENTHESES));
                    break;

                case '{':
                case '}':
                    attrset(COLOR_PAIR(BRACKETS));
                    break;

                default: {
                    if (nowComment || nowConsecutiveComment) {
                        if (*p == '/') {
                            nowConsecutiveComment = false;
                            nowComment = false;
                        }
                        attrset(COLOR_PAIR(COMMENT));
                        break;
                    }

                    if (nowString) {
                        if (*p == '"' || *p == '\'') 
                            nowString = false;
                        attrset(COLOR_PAIR(SP_RESERVED));
                        break;
                    }


                    if (!isdigit(*p)) {
                        vector<Token> vec = initPredictiveTransform();
                        for (auto v : vec) {
                            if (isFunction(gBuf, gPageStart)) {
                                tokenPaint(&nowToken, &tokenCounter, 1, FUNCTION);
                                break;
                            }

                            if (split_token(gBuf, gPageEnd, v.word.c_str(), v.word.size()) ||
                                (nowToken == v.type && tokenCounter > 0)) {
                                    if (v.type == COMMENT)	nowComment = !nowComment;
                                    if (v.type == CONSECUTIVECOMMENT) nowConsecutiveComment = true;
                                tokenPaint(&nowToken, &tokenCounter, v.word.size(), v.type);
                                break;
                            }
                            attrset(COLOR_PAIR(NOMAL));
                        }
                    }
                    else attrset(COLOR_PAIR(NUM));
                    break;
                }
            }

            (*p == '\t') ? printw("    ") : addch(*p);

            x += *p == '\t' ? 4 - (x & 3) : 1;
        }

    }

	// 余白
    move(y, 0);
    
    while (y < h) { 
        drawInDir(finderSwitch, lineNumberString, y++);
        lineNumberString = "~\n";
        drawTildeAndFinder(&lineNumberString,  AllLineLength);
    }
    // 

    // カーソルの表示
    move(gRow, gCol+nowLineBuf);
    refresh(); 
}

