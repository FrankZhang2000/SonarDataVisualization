#ifndef PREPROCESSOR
#define PREPROCESSOR

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <mat.h>
#include "utils.h"
#include "settings.h"

using namespace std;

class DataPreprocessor {
public:
    double color_max, color_min;
    vector<Point> & data;
    DataPreprocessor(vector<Point> & d);
    void load_mat(const char * filenm, double thre, bool enhance);
    void load_txt(const char * filenm, double thre);
};

class NoiseFilter {
public:
    vector<int> point_cnt;
    int dist, thre;
    NoiseFilter(int _dist, int _thre);
    void set_filter(vector<Point> & v);
    bool is_noise(Point & p);
    void process_data(vector<Point> & raw, vector<Point> & denoised);
};

#endif