#ifndef OBJWRITER
#define OBJWRITER

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <cmath>
#include <mat.h>
#include "utils.h"
#include "settings.h"

using namespace std;

class DistComp {
public:
    bool operator()(const double & d1, const double & d2) const {
        return fabs(d1 - d2) < DIST_THRE;
    }
};

class OriginPoint {
public:
    double r, refl;
    OriginPoint(double _r, double _refl);
    bool operator<(const OriginPoint & p) const;
};

class ObjectWriter {
public:
    vector<ObjectPoint> & rawdata;
    vector<OriginPoint> data[OUTPUT_DIM_X][OUTPUT_DIM_Y];
    ObjectWriter(vector<ObjectPoint> & _rawdata, bool rm_near);
    void insert_object(const ObjectWriter & obj);
    void write_object_to_mat(string outdir);
    void write_object_to_txt(string outdir);
};

#endif