#ifndef FONT
#define FONT

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cstring>
#include <GL/glut.h>
#include "settings.h"

using namespace std;

class Character {
public:
    bool top_align;
    int width, height;
    int x_offset, y_offset;
    vector<pair<int, int> > bits;
    Character(bool _top_align, int _width, int _height, int _x_offset, int _y_offset);
};

class FontFormatter {
public:
    map<char, Character> char_map;
    void load_font(string filenm);
    void draw_text(int x, int y, const char * text);
};

#endif