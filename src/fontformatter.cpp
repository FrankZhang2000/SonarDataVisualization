#include "fontformatter.h"

Character::Character(bool _top_align, int _width, int _height, int _x_offset, int _y_offset) {
    top_align = _top_align;
    width = _width;
    height = _height;
    x_offset = _x_offset;
    y_offset = _y_offset;
}

void FontFormatter::load_font(string filenm) {
    ifstream infile(filenm.c_str());
    char c;
    while (infile >> c) {
        int width, height, x_offset, y_offset;
        bool top_align;
        infile >> width >> height;
        if (c == 'g' || c == 'j' || c == 'p' || c == 'q' || c == 'y')
            top_align = true;
        else
            top_align = false;
        if (width < CHAR_W) {
            x_offset = (CHAR_W - width) / 2;
            width = CHAR_W;
        }
        else
            x_offset = 0;
        if (c == '(' || c == ')' || c == '-')
            y_offset = (CHAR_H - height) / 2;
        else
            y_offset = 0;
        Character character(top_align, width, height, x_offset, y_offset);
        int x, y;
        while (infile >> x >> y) {
            if (x == -1 && y == -1)
                break;
            character.bits.push_back(make_pair(x, y));
        }
        char_map.insert(make_pair(c, character));
    }
    Character blankspace(false, CHAR_W, CHAR_H, 0, 0);
    char_map.insert(make_pair(' ', blankspace));
}

void FontFormatter::draw_text(int x, int y, const char * text) {
    int len = strlen(text);
    glViewport(0, WINDOW_H + BAR_H, WINDOW_W, BAR_H);
    glPushMatrix();
    glLoadIdentity();
    glPointSize(1.0);
    glColor3f(0.0, 1.0, 1.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < len; i++) {
        char c = text[i];
        char_map.find(c);
        map<char, Character>::iterator itm;
        if ((itm = char_map.find(c)) == char_map.end()) {
            cerr << "Error: Illegal character '" << c << "' encountered!" << endl;
            glEnd();
            glPopMatrix();
            exit(1);
        }
        Character & character = itm->second;
        vector<pair<int, int> >::iterator itv;
        for (itv = character.bits.begin(); itv != character.bits.end(); itv++) {
            if (character.top_align) {
                glVertex2f(
                    (x + character.x_offset + itv->first) * SCALE_W, 
                    (y + character.y_offset + CHAR_HALF - itv->second) * SCALE_H
                );
            }
            else {
                glVertex2f(
                    (x + character.x_offset + itv->first) * SCALE_W, 
                    (y + character.y_offset + character.height - itv->second) * SCALE_H
                );
            }
        }
        x += (character.width + 2);
    }
    glEnd();
    glPopMatrix();
}