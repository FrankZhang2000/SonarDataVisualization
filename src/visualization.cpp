#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <memory.h>
#include <unistd.h>
#include <getopt.h>
#include <GL/glut.h>
#include "settings.h"
#include "utils.h"
#include "classifier.h"
#include "fontformatter.h"
#include "preprocessor.h"
#include "objwriter.h"

using namespace std;

int prev_mousex, prev_mousey;
bool mouse_pressed;
GLfloat zoom_ratio, preset_zoom_ratio;
vector<Point> rawdata, curr_data;
GLfloat rotate_mat[16];
int move_x, move_y, move_z;
int viewpoint_x, viewpoint_y, viewpoint_z;
bool use_preset_pointcnt;
int min_pointcnt, preset_pointcnt;
bool use_preset_noisethre;
int noisethre, preset_noisethre;
GLfloat color_thre, preset_color_thre;
GLfloat color_min, color_max;
int group_count;
int selbar_len;
Classifier classifier(DIST_INIT, POINTCNT_INIT);
bool display_groupinfo;
bool use_preset_thre, use_preset_ratio, use_preset_dist;
bool display_noise;
bool display_mulpoints;
bool preset_shownoise;
bool preset_noenhance;
bool preset_savedir;
string savedir, file_name;
bool print_groupinfo;
GLfloat group_dist, preset_dist;
int display_groupid;
FontFormatter formatter;
GLfloat curr_zmin;
GLfloat thre_actual_min;
int curr_input_groupid;
string fontfilenm;

enum input_format {
    txt, mat, none
} format;

enum arg_types {
    arg_txt, arg_mat, arg_thre, arg_ratio, 
    arg_dist, arg_point, arg_help, arg_noise, 
    arg_enhance, arg_save, arg_noisethre, arg_font
};

GLfloat front_mat[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, -1, 0,
    0, 0, 0, 1
};

GLfloat side_mat[16] = {
    0, 0, 1, 0,
    0, 1, 0, 0,
    1, 0, 0, 0,
    0, 0, 0, 1
};

GLfloat up_mat[16] = {
    1, 0, 0, 0,
    0, 0, -1, 0,
    0, -1, 0, 0,
    0, 0, 0, 1
};

GLfloat color_map[COLORMAP_LEN][3] = {
    1.000000, 1.000000, 0.800000,
    1.000000, 0.970279, 0.727348,
    1.000000, 0.940557, 0.654696,
    0.998968, 0.908772, 0.584107,
    0.997317, 0.875748, 0.514757,
    0.996078, 0.834881, 0.445408,
    0.996078, 0.770485, 0.376058,
    0.996078, 0.706089, 0.306708,
    0.994634, 0.644582, 0.274923,
    0.992982, 0.583488, 0.248504,
    0.991331, 0.500929, 0.220433,
    0.989680, 0.396904, 0.190712,
    0.983075, 0.295150, 0.161816,
    0.941796, 0.209288, 0.138700,
    0.900516, 0.123426, 0.115583,
    0.843137, 0.069763, 0.122188,
    0.780392, 0.026832, 0.138700,
    0.703406, 0.000000, 0.149020,
    0.602683, 0.000000, 0.149020,
    0.501961, 0.000000, 0.149020
};

void print_usage() {
    cout << "OVERVIEW: a simple program for underwater sonar data visualization" << endl;
    cout << "USAGE: visualization [options]" << endl;
    cout << "OPTIONS: " << endl;
    cout << "  --help/-h\t\t\tPrint help message" << endl;
    cout << "  --txt/-t <file_name>\t\tUse file <file_name> as input in .txt format" << endl;
    cout << "  --mat/-m <file_name>\t\tUse file <file_name> as input in .mat format" << endl;
    cout << "  --font/-f <file_name>\t\tUse file <file_name> as font bitmap file" << endl;
    cout << "  --thre <threshold>\t\tSet default reflection threshold to <threshold>" << endl;
    cout << "  --ratio <zoom_ratio>\t\tSet default zooming ratio to <zoom_ratio>" << endl;
    cout << "  --dist <group_dist>\t\tSet default grouping distance to <group_dist>" << endl;
    cout << "  --groupsize <point_num>\tSet default minimum group size to <point_num>" << endl;
    cout << "  --noisethre <point_num>\tSet default noise threshold to <point_num>" << endl;
    cout << "  --savedir <dir>\t\tSet object saving directory to <dir>" << endl;
    cout << "  --shownoise\t\t\tDisable noise detector and show all noises" << endl;
    cout << "  --noenhance\t\t\tDisable resolution enhancement" << endl;
}

void print_keyboard_usage() {
    cout << "KEYBOARD USAGE:" << endl;
    cout << "  Key H\t\t\tPrint keyboard usage" << endl;
    cout << "  Key A,D\t\tMove viewpoint along x axis" << endl;
    cout << "  Key W,S\t\tMove viewpoint along y axis" << endl;
    cout << "  Key Q,E\t\tAdjust grouping distance" << endl;
    cout << "  Key Z,X\t\tAdjust minimum group size" << endl;
    cout << "  Key C,V\t\tAdjust noise threshold" << endl;
    cout << "  Key O\t\t\tSave data of selected group in .mat format" << endl;
    cout << "  Key T\t\t\tSave data of selected group in .txt format" << endl;
    cout << "  Key P\t\t\tEnable/Disable print of group metrics" << endl;
    cout << "  Key N\t\t\tEnable/Disable display of noise" << endl;
    cout << "  Key M\t\t\tEnable/Disable resolution enhancement" << endl;
    cout << "  Key UP,DOWN\t\tAdjust zooming ratio" << endl;
    cout << "  Key LEFT,RIGHT\tAdjust reflection threshold" << endl;
    cout << "  Key <,>\t\tSelect previous/next group for inspection" << endl;
    cout << "  Key Number<N>\t\tAppend number <N> to the back of <input_id>" << endl;
    cout << "  Key -\t\t\tRemove the last number of <input_id>" << endl;
    cout << "  Key =\t\t\tSelect group <input_id> for inspection" << endl;
}

void load_data(GLfloat thre) {
    vector<Point> noise_data;
    vector<Point>::iterator it;
    for (it = rawdata.begin(); it != rawdata.end(); it++) {
        if (it->color >= thre)
            noise_data.push_back(*it);
    }
    NoiseFilter filter(FILTER_DIST, noisethre);
    filter.set_filter(noise_data);
    curr_data = vector<Point>();
    GLfloat z_min = 0;
    if (preset_shownoise) {
        for (it = noise_data.begin(); it != noise_data.end(); it++) {
            if (filter.is_noise(*it))
                it->group_id = -1;
            curr_data.push_back(*it);
            if (it->z < z_min)
                z_min = it->z;
        }
    }
    else {
        for (it = noise_data.begin(); it != noise_data.end(); it++) {
            if (!filter.is_noise(*it)) {
                curr_data.push_back(*it);
                if (it->z < z_min)
                    z_min = it->z;
            }
        }
    }
    curr_zmin = z_min;
    if (curr_data.size() > 0) {
        color_min = curr_data.begin()->color;
        color_max = curr_data.rbegin()->color;
    }
    else {
        cerr << "Error: Data length is zero!" << endl;
        exit(1);
    }
}

void reset_others() {
    if (use_preset_thre)
        color_thre = preset_color_thre;
    else
        color_thre = THRE_INIT;
    if (use_preset_dist)
        group_dist = preset_dist;
    else
        group_dist = DIST_INIT;
    if (use_preset_noisethre)
        noisethre = preset_noisethre;
    else
        noisethre = FILTER_THRE_INIT;
    if (use_preset_pointcnt)
        min_pointcnt = preset_pointcnt;
    else
        min_pointcnt = POINTCNT_INIT;
}

void reset_moves() {
    memset(rotate_mat, 0, sizeof(rotate_mat));
    rotate_mat[0] = 1;
    rotate_mat[5] = 1;
    rotate_mat[10] = -1;
    rotate_mat[15] = 1;
    if (use_preset_ratio)
        zoom_ratio = preset_zoom_ratio;
    else
        zoom_ratio = ZOOM_INIT;
    move_x = move_y = move_z = 0;
    viewpoint_x = 0;
    viewpoint_y = 0;
    viewpoint_z = curr_zmin;
    display_groupinfo = false;
}

void set_color(GLfloat color) {
    int index = round((color - color_min) * COLORMAP_LEN / (color_max - color_min));
    if (index < 0)
        index = 0;
    else if (index > COLORMAP_LEN - 1)
        index = COLORMAP_LEN - 1;
    glColor3f(color_map[index][0], color_map[index][1], color_map[index][2]);
}

void set_color(int group_id) {
    int index = (group_id + 1) * COLORMAP_LEN / (group_count + 1);
    glColor3f(color_map[index][0], color_map[index][1], color_map[index][2]);
}

GLfloat get_size(GLfloat level, bool in_mainview) {
    GLfloat size = (level - color_min) * (MAX_POINT_SIZE - MIN_POINT_SIZE) / (color_max - color_min) + MIN_POINT_SIZE;
    if (in_mainview)
        return size * zoom_ratio;
    else
        return size;
}

void draw_arrow() {
    glBegin(GL_LINES);
    glLineWidth(5.0);
    glColor3f(0.0, 1.0, 1.0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, -100.0);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex3f(-10.0, 0, -100.0);
    glVertex3f(10.0, 0, -100.0);
    glVertex3f(0, 0, -115.0);
    glVertex3f(0, -10.0, -100.0);
    glVertex3f(0, 10.0, -100.0);
    glVertex3f(0, 0, -115.0);
    glEnd();
}

void draw_seplines() {
    glColor3f(0.0, 1.0, 1.0);
    glLineWidth(1.0);
    glLoadIdentity();
    glViewport(WINDOW_W - 1, 0, 2, WINDOW_H + 2 * BAR_H);
    glBegin(GL_LINES);
    glVertex2f(1, -NRANGE * K);
    glVertex2f(1, NRANGE * K);
    glEnd();
    glViewport(0, BAR_H - 1, WINDOW_W, 2);
    glBegin(GL_LINES);
    glVertex2f(-(NRANGE * K * SUBWINDOW_W / SUBWINDOW_H), 0);
    glVertex2f(NRANGE * K * SUBWINDOW_W / SUBWINDOW_H, 0);
    glEnd();
    glViewport(0, WINDOW_H + BAR_H - 1, WINDOW_W, 2);
    glBegin(GL_LINES);
    glVertex2f(-(NRANGE * K * SUBWINDOW_W / SUBWINDOW_H), 0);
    glVertex2f(NRANGE * K * SUBWINDOW_W / SUBWINDOW_H, 0);
    glEnd();
    glLoadIdentity();
    glViewport(WINDOW_W, SUBWINDOW_H - 1, SUBWINDOW_W, 2);
    glBegin(GL_LINES);
    glVertex2f(-(NRANGE * K * SUBWINDOW_W / SUBWINDOW_H), 1);
    glVertex2f(NRANGE * K * SUBWINDOW_W / SUBWINDOW_H, 1);
    glEnd();
    glLoadIdentity();
    glViewport(WINDOW_W, 2 * SUBWINDOW_H - 1, SUBWINDOW_W, 2);
    glBegin(GL_LINES);
    glVertex2f(-(NRANGE * K * SUBWINDOW_W / SUBWINDOW_H), 1);
    glVertex2f(NRANGE * K * SUBWINDOW_W / SUBWINDOW_H, 1);
    glEnd();
}

void draw_selbar() {
    selbar_len = min(group_count, BAR_LEN);
    glPointSize(30);
    for (int i = 0; i < selbar_len; i++) {
        glLoadIdentity();
        glViewport(i * BAR_H, 0, BAR_H, BAR_H);
        set_color(i);
        glBegin(GL_POINTS);
        glVertex2f(0.0, 0.0);
        glEnd();
    }
    glLoadIdentity();
    glViewport(BAR_LEN * BAR_H, 0, BAR_H, BAR_H);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_POINTS);
    glVertex2f(0.0, 0.0);
    glEnd();
}

void init(const char * filenm) {
    color_min = 100;
    color_max = 0;
    mouse_pressed = false;
    if (preset_shownoise)
        display_noise = true;
    else
        display_noise = false;
    display_mulpoints = true;
    print_groupinfo = true;
    if (use_preset_thre)
        thre_actual_min = min((GLfloat)THRE_INIT, preset_color_thre);
    else
        thre_actual_min = THRE_INIT;
    DataPreprocessor preprocessor(rawdata);
    if (format == txt)
        preprocessor.load_txt(filenm, thre_actual_min);
    else if (format == mat)
        preprocessor.load_mat(filenm, thre_actual_min, !preset_noenhance);
    else {
        cerr << "Error: Input file is not correctly specified!" << endl;
        print_usage();
        exit(1);
    }
    reset_others();
    if (use_preset_thre)
        load_data(preset_color_thre);
    else
        load_data(THRE_INIT);
    reset_moves();
    classifier = Classifier(group_dist, min_pointcnt);
    group_count = classifier.group_data(curr_data);
    formatter = FontFormatter();
    formatter.load_font(fontfilenm);
    glClearColor(0.4, 0.4, 0.4, 1.0);
}

void draw_points(bool in_mainview) {
    vector<Point>::iterator it;
    GLfloat curr_size = POINT_SIZE_STEP;
    glPointSize(curr_size);
    glBegin(GL_POINTS);
    for (it = curr_data.begin(); it != curr_data.end(); it++) {
        if (!display_mulpoints && !it->is_origin)
            continue;
        GLfloat size = get_size(it->color, in_mainview);
        if (size >= curr_size + POINT_SIZE_STEP) {
            curr_size += POINT_SIZE_STEP;
            glEnd();
            glPointSize(curr_size);
            glBegin(GL_POINTS);
        }
        if (it->group_id != -1 || display_noise) {
            set_color(it->group_id);
            glVertex3f(it->x, it->y, it->z);
        }
    }
    glEnd();
}

void draw_text() {
    if (display_groupinfo) {
        Group & group = classifier.group_list[display_groupid];
        char text1[200];
        sprintf(
            text1, 
            "Group: %d, Position: (%.3f, %.3f, %.3f)", 
            group.id + 1,
            group.center_x, group.center_y, group.center_z
        );
        formatter.draw_text(-615, 18, text1);
        char text2[200];
        sprintf(
            text2, 
            "Direction: (%.3f, %.3f, %.3f), Length: %.3f m", 
            group.direction.x, group.direction.y, group.direction.z, 
            group.length
        );
        formatter.draw_text(-615, -51, text2);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    glViewport(0, BAR_H, WINDOW_W, WINDOW_H);
    glScalef(zoom_ratio, zoom_ratio, zoom_ratio);
    glTranslatef(move_x, move_y, move_z);
    glMultMatrixf(rotate_mat);
    glTranslatef(-viewpoint_x, -viewpoint_y, -viewpoint_z);
    draw_arrow();
    draw_points(true);
    glLoadIdentity();
    glViewport(WINDOW_W, 0, SUBWINDOW_W, SUBWINDOW_H);
    glMultMatrixf(up_mat);
    glTranslatef(0.0, 0.0, -curr_zmin);
    draw_arrow();
    draw_points(false);
    glLoadIdentity();
    glViewport(WINDOW_W, SUBWINDOW_H, SUBWINDOW_W, SUBWINDOW_H);
    glMultMatrixf(side_mat);
    glTranslatef(0.0, 0.0, -curr_zmin);
    draw_arrow();
    draw_points(false);
    glLoadIdentity();
    glViewport(WINDOW_W, 2 * SUBWINDOW_H, SUBWINDOW_W, SUBWINDOW_H);
    glMultMatrixf(front_mat);
    glTranslatef(0.0, 0.0, -curr_zmin);
    draw_arrow();
    draw_points(false);
    draw_selbar();
    draw_seplines();
    draw_text();
    glPopMatrix();
    glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h) {
    if (w <= 0 || h <= 0) {
        cerr << "Error: Window size must be positive" << endl;
        exit(1);
    }
    glViewport(0, BAR_H, WINDOW_W, WINDOW_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLfloat ratio = WINDOW_W / WINDOW_H;
    if (ratio <= 1.0) {
        glOrtho(
            -NRANGE * K, NRANGE * K,
            -NRANGE * K / ratio, NRANGE * K / ratio,
            -NRANGE * K, NRANGE * K
        );
    }
    else {
        glOrtho(
            -NRANGE * K * ratio, NRANGE * K * ratio,
            -NRANGE * K, NRANGE * K,
            -NRANGE * K, NRANGE * K
        );
    }
    glViewport(WINDOW_W, 0, SUBWINDOW_W, SUBWINDOW_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (ratio <= 1.0) {
        glOrtho(
            -NRANGE * K, NRANGE * K,
            -NRANGE * K / ratio, NRANGE * K / ratio,
            -NRANGE * K, NRANGE * K
        );
    }
    else {
        glOrtho(
            -NRANGE * K * ratio, NRANGE * K * ratio,
            -NRANGE * K, NRANGE * K,
            -NRANGE * K, NRANGE * K
        );
    }
    glViewport(WINDOW_W, SUBWINDOW_H, SUBWINDOW_W, SUBWINDOW_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (ratio <= 1.0) {
        glOrtho(
            -NRANGE * K, NRANGE * K,
            -NRANGE * K / ratio, NRANGE * K / ratio,
            -NRANGE * K, NRANGE * K
        );
    }
    else {
        glOrtho(
            -NRANGE * K * ratio, NRANGE * K * ratio,
            -NRANGE * K, NRANGE * K,
            -NRANGE * K, NRANGE * K
        );
    }
    glViewport(WINDOW_W, 2 * SUBWINDOW_H, SUBWINDOW_W, SUBWINDOW_H);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (ratio <= 1.0) {
        glOrtho(
            -NRANGE * K, NRANGE * K,
            -NRANGE * K / ratio, NRANGE * K / ratio,
            -NRANGE * K, NRANGE * K
        );
    }
    else {
        glOrtho(
            -NRANGE * K * ratio, NRANGE * K * ratio,
            -NRANGE * K, NRANGE * K,
            -NRANGE * K, NRANGE * K
        );
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void arcball_rotate(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
    GLfloat work_size = (ARCBALL_R / NRANGE) * min(WINDOW_W, WINDOW_H);
    GLfloat r_x1, r_y1, r_z1, r_x2, r_y2, r_z2, x, y, z;
    r_x1 = x1 - WINDOW_W;
    r_y1 = WINDOW_H - (y1 - BAR_H);
    if (square(r_x1) + square(r_y1) > square(work_size))
        return;
    r_z1 = sqrt(square(work_size) - square(r_x1) - square(r_y1));
    r_x2 = x2 - WINDOW_W;
    r_y2 = WINDOW_H - (y2 - BAR_H);
    if (square(r_x2) + square(r_y2) > square(work_size))
        return;
    r_z2 = sqrt(square(work_size) - square(r_x2) - square(r_y2));
    x = r_y1 * r_z2 - r_z1 * r_y2;
    y = r_z1 * r_x2 - r_x1 * r_z2;
    z = r_x1 * r_y2 - r_y1 * r_x2;
    GLfloat div = sqrt(square(x) + square(y) + square(z));
    x /= div;
    y /= div;
    z /= div;
    GLfloat dist = sqrt(square(r_x1 - r_x2) + square(r_y1 - r_y2) + square(r_z1 - r_z2));
    GLfloat angle = -2 * asin(dist / (2.0 * work_size));
    GLfloat mat[16] = {0};
    mat[0] = x * x + (1 - x * x) * cos(angle);
    mat[1] = x * y * (1 - cos(angle)) - z * sin(angle);
    mat[2]= x * z * (1 - cos(angle)) + y * sin(angle);
    mat[4] = x * y * (1 - cos(angle)) + z * sin(angle);
    mat[5] = y * y + (1 - y * y) * cos(angle);
    mat[6] = y * z * (1 - cos(angle)) - x * sin(angle);
    mat[8] = x * z * (1 - cos(angle)) - y * sin(angle);
    mat[9] = y * z * (1 - cos(angle)) + x * sin(angle);
    mat[10] = z * z + (1 - z * z) * cos(angle);
    mat[15] = 1;
    glLoadMatrixf(mat);
    glMultMatrixf(rotate_mat);
    glGetFloatv(GL_MODELVIEW_MATRIX, rotate_mat);
    glLoadIdentity();
    glGetFloatv(GL_MODELVIEW_MATRIX, mat);
    glutPostRedisplay();
}

void set_view(int group_id) {
    viewpoint_x = round(classifier.group_list[group_id].center_x);
    viewpoint_y = round(classifier.group_list[group_id].center_y);
    viewpoint_z = round(classifier.group_list[group_id].center_z);
    zoom_ratio = NRANGE * K / (classifier.group_list[group_id].length + 1);
    if (zoom_ratio > ZOOM_UPTHRES)
        zoom_ratio = ZOOM_UPTHRES;
    else if (zoom_ratio < ZOOM_DOWNTHRES)
        zoom_ratio = ZOOM_DOWNTHRES;
}

void mouse_click(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (x < WINDOW_W) {
            if (y < WINDOW_H + BAR_H && y > BAR_H) {
                if (state == GLUT_DOWN && !mouse_pressed) {
                    mouse_pressed = true;
                    prev_mousex = x;
                    prev_mousey = y;
                }
                else if (state == GLUT_UP)
                    mouse_pressed = false;
            }
            else if (y > WINDOW_H + BAR_H) {
                if (state == GLUT_DOWN && !mouse_pressed) {
                    int selected = x / BAR_H;
                    if (selected < selbar_len) {
                        reset_moves();
                        classifier.calc_groupinfo(selected);
                        if (print_groupinfo)
                            classifier.group_list[selected].print_metrics();
                        set_view(selected);
                        display_groupinfo = true;
                        display_groupid = selected;
                        glutPostRedisplay();
                        if (curr_input_groupid != 0) {
                            curr_input_groupid = 0;
                            cout << "Current Group ID from input is set to 0" << endl;
                        }
                    }
                    else if (selected == BAR_LEN) {
                        reset_others();
                        if (use_preset_thre)
                            load_data(preset_color_thre);
                        else
                            load_data(THRE_INIT);
                        reset_moves();
                        classifier = Classifier(group_dist, min_pointcnt);
                        group_count = classifier.group_data(curr_data);
                        glutPostRedisplay();
                        if (curr_input_groupid != 0) {
                            curr_input_groupid = 0;
                            cout << "Current Group ID from input is set to 0" << endl;
                        }
                    }
                }
                else if (state == GLUT_UP)
                    mouse_pressed = false;
            }
        }
        else {
            mouse_pressed = false;
            if (state == GLUT_DOWN) {
                mouse_pressed = false;
                if (y < SUBWINDOW_H) {
                    reset_moves();
                    memcpy(rotate_mat, front_mat, sizeof(front_mat));
                }
                else if (y < 2 * SUBWINDOW_H) {
                    reset_moves();
                    memcpy(rotate_mat, side_mat, sizeof(side_mat));
                }
                else {
                    reset_moves();
                    memcpy(rotate_mat, up_mat, sizeof(up_mat));
                }
                glutPostRedisplay();
            }
        }
    }
}

void mouse_drag(int x, int y) {
    if (mouse_pressed) {
        arcball_rotate(prev_mousex, prev_mousey, x, y);
        prev_mousex = x;
        prev_mousey = y;
    }
}

void keyboard_zoom(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_UP:
        if (zoom_ratio + ZOOM_SPEED <= ZOOM_UPTHRES) {
            zoom_ratio += ZOOM_SPEED;
            glutPostRedisplay();
        }
        else
            cout << "Info: Zooming ratio reaches upper limit!" << endl;
        break;
    case GLUT_KEY_DOWN:
        if (zoom_ratio - ZOOM_SPEED >= ZOOM_DOWNTHRES) {
            zoom_ratio -= ZOOM_SPEED;
            glutPostRedisplay();
        }
        else
            cout << "Info: Zooming ratio reaches lower limit!" << endl;
        break;
    case GLUT_KEY_RIGHT:
        if (color_thre + THRE_STEP <= THRE_MAX) {
            color_thre += THRE_STEP;
            load_data(color_thre);
            display_groupinfo = false;
            classifier = Classifier(group_dist, min_pointcnt);
            group_count = classifier.group_data(curr_data);
            glutPostRedisplay();
        }
        else
            cout << "Info: Reflection threshold reaches upper limit!" << endl;
        if (curr_input_groupid != 0) {
            curr_input_groupid = 0;
            cout << "Current Group ID from input is set to 0" << endl;
        }
        break;
    case GLUT_KEY_LEFT:
        if (color_thre - THRE_STEP >= max(thre_actual_min, (GLfloat)THRE_MIN)) {
            color_thre -= THRE_STEP;
            load_data(color_thre);
            display_groupinfo = false;
            classifier = Classifier(group_dist, min_pointcnt);
            group_count = classifier.group_data(curr_data);
            glutPostRedisplay();
        }
        else
            cout << "Info: Reflection threshold reaches lower limit!" << endl;
        if (curr_input_groupid != 0) {
            curr_input_groupid = 0;
            cout << "Current Group ID from input is set to 0" << endl;
        }
        break;
    default:
        break;
    }
}

void keyboard_move(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':
        if (move_y + MOVE_SPEED <= MOVE_POSTHRES) {
            move_y += MOVE_SPEED;
            glutPostRedisplay();
        }
        else
            cout << "Info: X coord. of viewpoint reaches upper limit!" << endl;
        break;
    case 'a':
        if (move_x - MOVE_SPEED >= MOVE_NEGTHRES) {
            move_x -= MOVE_SPEED;
            glutPostRedisplay();
        }
        else
            cout << "Info: X coord. of viewpoint reaches lower limit!" << endl;
        break;
    case 's':
        if (move_y - MOVE_SPEED >= MOVE_NEGTHRES) {
            move_y -= MOVE_SPEED;
            glutPostRedisplay();
        }
        else
            cout << "Info: Y coord. of viewpoint reaches lower limit!" << endl;
        break;
    case 'd':
        if (move_x + MOVE_SPEED <= MOVE_POSTHRES) {
            move_x += MOVE_SPEED;
            glutPostRedisplay();
        }
        else
            cout << "Info: Y coord. of viewpoint reaches upper limit!" << endl;
        break;
    case 'z':
        if (min_pointcnt - POINTCNT_STEP >= POINTCNT_DOWNTHRE) {
            min_pointcnt -= POINTCNT_STEP;
            load_data(color_thre);
            display_groupinfo = false;
            classifier = Classifier(group_dist, min_pointcnt);
            group_count = classifier.group_data(curr_data);
            glutPostRedisplay();
        }
        else
            cout << "Info: Min. point count reaches lower limit!" << endl;
        if (curr_input_groupid != 0) {
            curr_input_groupid = 0;
            cout << "Current Group ID from input is set to 0" << endl;
        }
        break;
    case 'x':
        if (min_pointcnt + POINTCNT_STEP <= POINTCNT_UPTHRE) {
            min_pointcnt += POINTCNT_STEP;
            load_data(color_thre);
            display_groupinfo = false;
            classifier = Classifier(group_dist, min_pointcnt);
            group_count = classifier.group_data(curr_data);
            glutPostRedisplay();
        }
        else
            cout << "Info: Min. point count reaches upper limit!" << endl;
        if (curr_input_groupid != 0) {
            curr_input_groupid = 0;
            cout << "Current Group ID from input is set to 0" << endl;
        }
        break;
    case 'c':
        if (noisethre - FILTER_THRE_STEP >= FILTER_THRE_MIN) {
            noisethre -= FILTER_THRE_STEP;
            load_data(color_thre);
            display_groupinfo = false;
            classifier = Classifier(group_dist, min_pointcnt);
            group_count = classifier.group_data(curr_data);
            glutPostRedisplay();
        }
        else
            cout << "Info: Noise threshold reaches lower limit!" << endl;
        if (curr_input_groupid != 0) {
            curr_input_groupid = 0;
            cout << "Current Group ID from input is set to 0" << endl;
        }
        break;
    case 'v':
        if (noisethre + FILTER_THRE_STEP <= FILTER_THRE_MAX) {
            noisethre += FILTER_THRE_STEP;
            load_data(color_thre);
            display_groupinfo = false;
            classifier = Classifier(group_dist, min_pointcnt);
            group_count = classifier.group_data(curr_data);
            glutPostRedisplay();
        }
        else
            cout << "Info: Noise threshold reaches upper limit!" << endl;
        if (curr_input_groupid != 0) {
            curr_input_groupid = 0;
            cout << "Current Group ID from input is set to 0" << endl;
        }
        break;
    case 'q':
        if (group_dist - DIST_STEP >= DIST_MIN) {
            group_dist -= DIST_STEP;
            display_groupinfo = false;
            load_data(color_thre);
            classifier = Classifier(group_dist, min_pointcnt);
            group_count = classifier.group_data(curr_data);
            glutPostRedisplay();
        }
        else
            cout << "Info: Grouping distance reaches lower limit!" << endl;
        if (curr_input_groupid != 0) {
            curr_input_groupid = 0;
            cout << "Current Group ID from input is set to 0" << endl;
        }
        break;
    case 'e':
        if (group_dist + DIST_STEP <= DIST_MAX) {
            group_dist += DIST_STEP;
            display_groupinfo = false;
            load_data(color_thre);
            classifier = Classifier(group_dist, min_pointcnt);
            group_count = classifier.group_data(curr_data);
            glutPostRedisplay();
        }
        else
            cout << "Info: Grouping distance reaches upper limit!" << endl;
        if (curr_input_groupid != 0) {
            curr_input_groupid = 0;
            cout << "Current Group ID from input is set to 0" << endl;
        }
        break;
    case 'n':
        display_noise = !display_noise;
        glutPostRedisplay();
        break;
    case 'm':
        if (preset_noenhance)
            cerr << "Info: Feature disabled due to --noenhance option!" << endl;
        else {
            display_mulpoints = !display_mulpoints;
            glutPostRedisplay();
        }
        break;
    case 'p':
        print_groupinfo = !print_groupinfo;
        break;
    case 'o':
        if (!preset_savedir)
            cerr << "Info: Feature disabled due to no --savedir option!" << endl;
        else {
            if (display_groupinfo) {
                char dir[100];
                sprintf(dir, "%s%s_%d.mat", savedir.c_str(), file_name.c_str(), display_groupid + 1);
                string outdir = dir;
                ObjectWriter writer(classifier.group_list[display_groupid].origin_data, false);
                writer.write_object_to_mat(outdir);
            }
            else
                cerr << "Info: No selected group!" << endl;
        }
        break;
    case 't':
        if (!preset_savedir)
            cerr << "Info: Feature disabled due to no --savedir option!" << endl;
        else {
            if (display_groupinfo) {
                char dir[100];
                sprintf(dir, "%s%s_%d.txt", savedir.c_str(), file_name.c_str(), display_groupid + 1);
                string outdir = dir;
                ObjectWriter writer(classifier.group_list[display_groupid].origin_data, false);
                writer.write_object_to_txt(outdir);
            }
            else
                cerr << "Info: No selected group!" << endl;
        }
        break;
    case 'h':
        print_keyboard_usage();
        break;
    case ',':
        if (display_groupinfo) {
            if (display_groupid - 1 >= 0) {
                reset_moves();
                display_groupid--;
                classifier.calc_groupinfo(display_groupid);
                if (print_groupinfo)
                    classifier.group_list[display_groupid].print_metrics();
                set_view(display_groupid);
                display_groupinfo = true;
                glutPostRedisplay();
            }
            else
                cerr << "Info: Group ID out of range!" << endl;
        }
        else
            cerr << "Info: No selected group!" << endl;
        break;
    case '.':
        if (display_groupinfo) {
            if (display_groupid + 1 < group_count) {
                reset_moves();
                display_groupid++;
                classifier.calc_groupinfo(display_groupid);
                if (print_groupinfo)
                    classifier.group_list[display_groupid].print_metrics();
                set_view(display_groupid);
                display_groupinfo = true;
                glutPostRedisplay();
            }
            else
                cerr << "Info: Group ID out of range!" << endl;
        }
        else
            cerr << "Info: No selected group!" << endl;
        break;
    case '-':
        curr_input_groupid /= 10;
        if (curr_input_groupid != 0)
            cout << "Current Group ID from input: " << curr_input_groupid << endl;
        else
            cout << "Current Group ID from input: " << endl;
        break;
    case '=':
        if (curr_input_groupid <= group_count && curr_input_groupid > 0) {
            reset_moves();
            classifier.calc_groupinfo(curr_input_groupid - 1);
            if (print_groupinfo)
                classifier.group_list[curr_input_groupid - 1].print_metrics();
            set_view(curr_input_groupid - 1);
            display_groupinfo = true;
            display_groupid = curr_input_groupid - 1;
            glutPostRedisplay();
        }
        else
            cerr << "Info: Group ID out of range!" << endl;
        curr_input_groupid = 0;
        cout << "Current Group ID from input is set to 0" << endl;
        break;
    default:
        if (key >= '0' && key <= '9') {
            if (curr_input_groupid < 1000)
                curr_input_groupid = curr_input_groupid * 10 + (key - '0');
            if (curr_input_groupid != 0)
                cout << "Current Group ID from input: " << curr_input_groupid << endl;
            else
                cout << "Current Group ID from input: " << endl;
        }
        break;
    }
}

void display_data(const char * filenm) {
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_W + SUBWINDOW_W, WINDOW_H + 2 * BAR_H);
    glutCreateWindow("UnderWater Sonar Data Visualization");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse_click);
    glutMotionFunc(mouse_drag);
    glutSpecialFunc(keyboard_zoom);
    glutKeyboardFunc(keyboard_move);
    init(filenm);
    glutMainLoop();
}

int main(int argc, char ** argv) {
    glutInit(&argc, argv);
    use_preset_thre = use_preset_ratio = use_preset_dist = false;
    use_preset_noisethre = use_preset_pointcnt = false;
    preset_shownoise = preset_noenhance = preset_savedir = false;
    curr_input_groupid = 0;
    format = none;
    fontfilenm = "./font/bitmap.txt";
    string filenm;
    static struct option long_options[] = {
        {"txt", required_argument, NULL, (int)arg_types::arg_txt}, 
        {"mat", required_argument, NULL, (int)arg_types::arg_mat}, 
        {"font", required_argument, NULL, (int)arg_types::arg_font}, 
        {"thre", required_argument, NULL, (int)arg_types::arg_thre}, 
        {"ratio", required_argument, NULL, (int)arg_types::arg_ratio},
        {"dist", required_argument, NULL, (int)arg_types::arg_dist},
        {"groupsize", required_argument, NULL, (int)arg_types::arg_point},
        {"noisethre", required_argument, NULL, (int)arg_types::arg_noisethre},
        {"help", no_argument, NULL, (int)arg_types::arg_help},
        {"shownoise", no_argument, NULL, (int)arg_types::arg_noise},
        {"noenhance", no_argument, NULL, (int)arg_types::arg_enhance},
        {"savedir", required_argument, NULL, (int)arg_types::arg_save}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "ht:m:f:", long_options, NULL)) != EOF) {
        switch (opt) {
        case 't':
        case (int)arg_types::arg_txt:
            format = txt;
            if (!optarg) {
                cerr << "Error: Input file is not correctly specified!" << endl;
                print_usage();
                exit(1);
            }
            filenm = optarg;
            break;
        case 'm':
        case (int)arg_types::arg_mat:
            format = mat;
            if (!optarg) {
                cerr << "Error: Input file is not correctly specified!" << endl;
                print_usage();
                exit(1);
            }
            filenm = optarg;
            break;
        case 'f':
        case (int)arg_types::arg_font:
            if (!optarg) {
                cerr << "Error: Font bitmap is not correctly specified!" << endl;
                print_usage();
                exit(1);
            }
            fontfilenm = optarg;
            break;
        case 'h':
        case (int)arg_types::arg_help:
            print_usage();
            exit(1);
            break;
        case (int)arg_types::arg_thre:
            use_preset_thre = true;
            preset_color_thre = stof(string(optarg));
            if (preset_color_thre < THRE_MIN) {
                printf("Info: Reflection threshold %.3f exceeds lower limit %.3f, it is set to %.3f.\n", 
                    preset_color_thre, (float)THRE_MIN, (float)THRE_MIN);
                preset_color_thre = THRE_MIN;
            }
            else if (preset_color_thre > THRE_MAX) {
                printf("Info: Reflection threshold %.3f exceeds upper limit %.3f, it is set to %.3f.\n", 
                    preset_color_thre, (float)THRE_MAX, (float)THRE_MAX);
                preset_color_thre = THRE_MAX;
            }
            break;
        case (int)arg_types::arg_ratio:
            use_preset_ratio = true;
            preset_zoom_ratio = stof(string(optarg));
            if (preset_zoom_ratio < ZOOM_DOWNTHRES) {
                printf("Info: Zooming ratio %.3f exceeds lower limit %.3f, it is set to %.3f.\n", 
                    preset_zoom_ratio, (float)ZOOM_DOWNTHRES, (float)ZOOM_DOWNTHRES);
                preset_zoom_ratio = ZOOM_DOWNTHRES;
            }
            else if (preset_zoom_ratio > ZOOM_UPTHRES) {
                printf("Info: Zooming ratio %.3f exceeds upper limit %.3f, it is set to %.3f.\n", 
                    preset_zoom_ratio, (float)ZOOM_UPTHRES, (float)ZOOM_UPTHRES);
                preset_zoom_ratio = ZOOM_UPTHRES;
            }
            break;
        case (int)arg_types::arg_dist:
            use_preset_dist = true;
            preset_dist = stof(string(optarg));
            if (preset_dist < DIST_MIN) {
                printf("Info: Grouping distance %.3f exceeds lower limit %.3f, it is set to %.3f.\n", 
                    preset_dist, (float)DIST_MIN, (float)DIST_MIN);
                preset_dist = DIST_MIN;
            }
            else if (preset_dist > DIST_MAX) {
                printf("Info: Grouping distance %.3f exceeds upper limit %.3f, it is set to %.3f.\n", 
                    preset_dist, (float)DIST_MAX, (float)DIST_MAX);
                preset_dist = DIST_MAX;
            }
            break;
        case (int)arg_types::arg_point:
            use_preset_pointcnt = true;
            preset_pointcnt = stof(string(optarg));
            if (preset_pointcnt < POINTCNT_DOWNTHRE) {
                printf("Info: Min. point count %d exceeds lower limit %d, it is set to %d\n", 
                    preset_pointcnt, POINTCNT_DOWNTHRE, POINTCNT_DOWNTHRE);
                preset_pointcnt = POINTCNT_DOWNTHRE;
            }
            else if (preset_pointcnt > POINTCNT_UPTHRE) {
                printf("Info: Min. point count %d exceeds upper limit %d, it is set to %d.\n", 
                    preset_pointcnt, POINTCNT_UPTHRE, POINTCNT_UPTHRE);
                preset_pointcnt = POINTCNT_UPTHRE;
            }
            break;
        case (int)arg_types::arg_noisethre:
            use_preset_noisethre = true;
            preset_noisethre = stof(string(optarg));
            if (preset_noisethre < FILTER_THRE_MIN) {
                printf("Info: Noise threshold %d exceeds lower limit %d, it is set to %d\n", 
                    preset_noisethre, FILTER_THRE_MIN, FILTER_THRE_MIN);
                preset_noisethre = FILTER_THRE_MIN;
            }
            else if (preset_noisethre > FILTER_THRE_MAX) {
                printf("Info: Noise threshold %d exceeds upper limit %d, it is set to %d\n", 
                    preset_noisethre, FILTER_THRE_MAX, FILTER_THRE_MAX);
                preset_noisethre = FILTER_THRE_MAX;
            }
            break;
        case (int)arg_types::arg_noise:
            preset_shownoise = true;
            break;
        case (int)arg_types::arg_enhance:
            preset_noenhance = true;
            break;
        case (int)arg_types::arg_save:
            preset_savedir = true;
            savedir = optarg;
            if (savedir.back() != '/')
                savedir.push_back('/');
            break;
        default:
            cerr << "Error: Wrong argument format!" << endl;
            print_usage();
            exit(1);
            break;
        }
    }
    if (format == none) {
        cerr << "Error: Input file is not correctly specified!" << endl;
        print_usage();
        exit(1);
    }
    if (preset_savedir) {
        string nm = filenm;
        int pos1 = nm.rfind('/');
        int pos2 = nm.rfind('.');
        if (pos1 == string::npos || pos2 == string::npos) {
            cerr << "Error: Failed to parse input file name!" << endl;
            exit(1);
        }
        file_name = nm.substr(pos1 + 1, pos2 - pos1 - 1);
    }
    display_data(filenm.c_str());
    return 0;
}