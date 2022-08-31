#ifndef CLASSIFIER
#define CLASSIFIER

#include <iostream>
#include <vector>
#include <cmath>
#include <climits>
#include <queue>
#include <algorithm>
#include <random>
#include "utils.h"
#include "settings.h"

using namespace std;

class Classifier;

class GridElement {
public:
    bool has_data;
    bool visited, visited_2, visited_3;
    bool on_edge;
    int group_id;
    float max_color;
    TriplePair<float> repr_point;
    TriplePair<float> center_point;
    GridElement();
};

class Group {
public:
    int id;
    int point_count, edge_pointcnt;
    bool has_info;
    float center_x, center_y, center_z;
    float avg_refl, min_refl, max_refl, dev_refl;
    float length, approx_length, edge_length;
    float act_width, max_width;
    float act_height, max_height;
    float bulk;
    TriplePair<float> direction;
    vector<ObjectPoint> edge_data;
    vector<ObjectPoint> origin_data;
    Group(int _id);
    void calc_metrics(Classifier & c);
    void print_metrics();
};

class GroupComp {
public:
    int pointcnt;
    GroupComp(int _pointcnt): pointcnt(_pointcnt) {}
    bool operator()(Group & g1, Group & g2) const {
        if (g1.point_count < pointcnt)
            return false;
        else if (g2.point_count < pointcnt)  
            return true;
        else {
            float op1 = sqrt(square(g1.center_x) + square(g1.center_y) + square(g1.center_z));
            float op2 = sqrt(square(g2.center_x) + square(g2.center_y) + square(g2.center_z));
            return op1 < op2;
        }
    }
};

class Classifier {
public:
    int x_min, x_max, y_min, y_max, z_min, z_max;
    int x_len, y_len, z_len;
    int group_count;
    float dist, pointcnt;
    GridElement * grid;
    vector<Group> group_list;
    Classifier(float _dist, float _pointcnt);
    ~Classifier();
    void init(vector<Point> & rawdata);
    int group_data(vector<Point> & rawdata);
    int get_offset(float x, float y, float z);
    int get_offset(int x, int y, int z);
    void alloc_groupid(int x, int y, int z, int id);
    void calc_groupinfo(int group_id);
};

#endif