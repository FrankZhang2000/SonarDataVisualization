#ifndef JOINOBJ
#define JOINOBJ

#include <iostream>
#include <vector>
#include <random>
#include <cstring>
#include <algorithm>
#include <functional>
#include "settings.h"
#include "utils.h"
using namespace std;

class ObjectTransformer {
public:
    float radius;
    TriplePair<float> center;
    vector<Point> data;
    vector<Point> noise;
    ObjectTransformer(vector<Point> & rawdata);
    void random_scale();
    void random_rotate();
    void random_move(float min_r, float max_r);
    void random_noise();
    static float randomf(float _min, float _max);
};

class ObjectJoiner {
public:
    int x_min, x_max, y_min, y_max, z_min, z_max;
    int x_len, y_len, z_len;
    float coll_dist;
    vector<ObjectPoint> joint_data;
    vector<ObjectTransformer> objects;
    vector<vector<Point> > & norm_data;
    vector<vector<Point> > & fixed_data;
    bool * grid;
    bool print_info;
    ObjectJoiner(vector<vector<Point> > & _norm_data, vector<vector<Point> > & _fixed_data, 
        bool _print_info, float _coll_dist);
    ~ObjectJoiner();
    void join_objects(float min_r, float max_r);
    bool center_available(TriplePair<float> & newcenter, float newradius);
    bool check_constraints(vector<Point> & points);
    void add_constraints(vector<Point> & points);
    void del_constraints(vector<Point> & points);
    int get_offset(float x, float y, float z);
};

#endif