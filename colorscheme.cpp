#include "eyo.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

short R;
short G;
short B;

int RGB_buf = 4;  // 215686274509802;

#define BACK 0

int color_gen(int r, int g, int b, int color) {
    init_color(color, r * RGB_buf, g * RGB_buf, b * RGB_buf);
    return color;
}

int color_gen_fromVec(vector<int> rgb, int color) {
    init_color(color, rgb[0] * RGB_buf, rgb[1] * RGB_buf, rgb[2] * RGB_buf);
    return color;
}

void jsonToScheme() {
	std::ifstream i("Nord.json");
	
	if (!i) {
		clear();
		refresh();
		printw("Can't find colorschem: \n");
		refresh();
		exit(1);
	}
	json j;
	i >> j;
	
	map<string, int> StrToNum = j["NumToStr"];
	map<string, vector<int>> colors = j["const"];
	map<string, vector<int>> statusBar = j["statusBar"];
	
	// ColorScheme
	for (auto itr = colors.begin(); itr != colors.end(); ++itr) {	
		init_pair(StrToNum[itr->first], color_gen_fromVec(itr->second, StrToNum[itr->first]), BACK);
	}

	// AirLine
	for (auto itr = statusBar.begin(); itr != statusBar.end(); ++itr) {	
		init_pair(StrToNum[itr->first], BACK, color_gen_fromVec(itr->second, StrToNum[itr->first]));
	}	
	i.close();
}

void backChange() {
     // Air line     
    init_pair(PARENTHESES, 0x4B, BACK);
    init_pair(BRACKETS, 0xE2, BACK);
    init_pair(SUBWIN, 0x69, 0x5A);
    init_pair(VARIABLE, 0x74, BACK);

	jsonToScheme();

    color_gen(46, 52, 64, BACK);

    return;
 }

void tokenPaint(int *nowToken, int *tokenCounter, const int len,
                const int attribute) {
    if (*tokenCounter == 0) *tokenCounter = len;
    (*tokenCounter)--;
    *nowToken = attribute;
    attrset(COLOR_PAIR(attribute));
}

