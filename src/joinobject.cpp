#include "joinobject.h"

bool comp(const Point & p1, const Point & p2) {
    if (p1.color == p2.color)
        return p1.r < p2.r;
    else
        return p1.color > p2.color;
}

ObjectTransformer::ObjectTransformer(vector<Point> & rawdata): data(rawdata), center(0.0, 0.0, 0.0) {
    vector<Point>::iterator it;
    for (it = data.begin(); it != data.end(); it++) {
        center.x += it->x;
        center.y += it->y;
        center.z += it->z;
    }
    center.x /= data.size();
    center.y /= data.size();
    center.z /= data.size();
}

void ObjectTransformer::random_scale() {
    float ratio_x = randomf(SCALE_DOWNTHRE, SCALE_UPTHRE);
    float ratio_y = randomf(SCALE_DOWNTHRE, SCALE_UPTHRE);
    float ratio_z = randomf(SCALE_DOWNTHRE, SCALE_UPTHRE);
    vector<Point>::iterator it;
    for (it = data.begin(); it != data.end(); it++) {
        it->x = center.x + ratio_x * (it->x - center.x);
        it->y = center.y + ratio_y * (it->y - center.y);
        it->z = center.z + ratio_z * (it->z - center.z);
    }
    radius = 0;
    for (it = data.begin(); it != data.end(); it++) {
        float dist = sqrt(square(it->x - center.x) + square(it->y - center.y) + square(it->z - center.z));
        if (dist > radius)
            radius = dist;
    }
}

void ObjectTransformer::random_rotate() {
    float angle_x = randomf(-M_PI, M_PI);
    float angle_y = randomf(-M_PI, M_PI);
    float angle_z = randomf(-M_PI, M_PI);
    float cos_beta, sin_beta;
    vector<Point>::iterator it;
    for (it = data.begin(); it != data.end(); it++) {
        float x0 = it->x - center.x;
        float y0 = it->y - center.y;
        float z0 = it->z - center.z;
        cos_beta = cos(angle_x);
        sin_beta = sin(angle_x);
        float x1 = x0;
        float y1 = cos_beta * y0 - sin_beta * z0;
        float z1 = sin_beta * y0 + cos_beta * z0;
        cos_beta = cos(angle_y);
        sin_beta = sin(angle_y);
        float x2 = cos_beta * x1 + sin_beta * z1;
        float y2 = y1;
        float z2 = -sin_beta * x1 + cos_beta * z1;
        cos_beta = cos(angle_z);
        sin_beta = sin(angle_z);
        float x3 = cos_beta * x2 - sin_beta * y2;
        float y3 = sin_beta * x2 + cos_beta * y2;
        float z3 = z2;
        it->x = x3 + center.x;
        it->y = y3 + center.y;
        it->z = z3 + center.z;
    }
}

void ObjectTransformer::random_move(float min_r, float max_r) {
    double viewangle_rad = (double)ANGLE_WIDTH / 180 * M_PI;
    double angle_thre = sin(viewangle_rad / 2);
    double x_angle_step = (2 * angle_thre / (OUTPUT_DIM_X - 1));
    double y_angle_step = (2 * angle_thre / (OUTPUT_DIM_Y - 1));
    float x_angle = randomf(-angle_thre, angle_thre);
    float y_angle = randomf(-angle_thre, angle_thre);
    float r = randomf(min_r, max_r);
    float new_x = r * (x_angle - (x_angle_step / 2));
    float new_y = r * (y_angle - (y_angle_step / 2));
    float new_z = -sqrt(square(r) - square(new_x) - square(new_y));
    vector<Point>::iterator it;
    for (it = data.begin(); it != data.end(); it++) {
        it->x += (new_x - center.x);
        it->y += (new_y - center.y);
        it->z += (new_z - center.z);
    }
    center.x = new_x;
    center.y = new_y;
    center.z = new_z;
}

void ObjectTransformer::random_noise() {
    double viewangle_rad = (double)ANGLE_WIDTH / 180 * M_PI;
    double angle_start = -sin(viewangle_rad / 2);
    double x_angle_step = (2 * sin(viewangle_rad / 2) / (OUTPUT_DIM_X - 1));
    double y_angle_step = (2 * sin(viewangle_rad / 2) / (OUTPUT_DIM_Y - 1));
    float min_r = Z_THRE;
    float min_refl = 1000.0;
    vector<Point>::iterator it;
    for (it = data.begin(); it != data.end(); it++) {
        it->r = sqrt(square(it->x) + square(it->y) + square(it->z));
        if (it->r < min_r)
            min_r = it->r;
        if (it->color < min_refl)
            min_refl = it->color;
    }
    for (int i = 0; i < N_NOISE; i++) {
        float r = randomf(min_r - R_DEV, min_r + R_DEV);
        float refl = randomf(min_refl - REFL_EXP - REFL_DEV, min_refl - REFL_EXP + REFL_DEV);
        int x_index = round(randomf(0.0, OUTPUT_DIM_X - 1));
        int y_index = round(randomf(0.0, OUTPUT_DIM_Y - 1));
        double x_angle = angle_start + x_index * x_angle_step;
        double y_angle = angle_start + y_index * y_angle_step;
        double x_convert = r * (x_angle - (x_angle_step / 2));
        double y_convert = r * (y_angle - (y_angle_step / 2));
        double z_convert = sqrt(square(r) - square(x_convert) - square(y_convert));
        noise.push_back(Point(x_convert, y_convert, z_convert, refl, r, false));
    }
}

float ObjectTransformer::randomf(float _min, float _max) {
    return ((float)(rand() % RAND_BASE) / RAND_BASE) * (_max - _min) + _min;
}

ObjectJoiner::ObjectJoiner(vector<vector<Point> > & _norm_data, vector<vector<Point> > & _fixed_data, 
        bool _print_info, float _coll_dist): norm_data(_norm_data), fixed_data(_fixed_data) {
    print_info = _print_info;
    coll_dist = _coll_dist;
    x_min = round(-X_THRE / coll_dist);
    x_max = round(X_THRE / coll_dist);
    y_min = round(-Y_THRE / coll_dist);
    y_max = round(Y_THRE / coll_dist);
    z_min = round(-Z_THRE / coll_dist);
    z_max = 0;
    x_len = x_max - x_min + 1;
    y_len = y_max - y_min + 1;
    z_len = z_max - z_min + 1;
    grid = new bool[x_len * y_len * z_len];
    memset(grid, false, x_len * y_len * z_len * sizeof(bool));
}

ObjectJoiner::~ObjectJoiner() {
    delete [] grid;
}

void ObjectJoiner::join_objects(float min_r, float max_r) {
    vector<vector<Point> >::iterator it;
    for (it = fixed_data.begin(); it != fixed_data.end(); it++) {
        if (print_info)
            printf("  Info: Trying to add Fixed Object %d...\n", (int)(it - fixed_data.begin() + 1));
        ObjectTransformer trans(*it);
        if (!check_constraints(trans.data))
            cerr << "Warning: Fixed Object overlapped or out of range!" << endl;
        if (print_info)
            printf("    SUCC: Move on to next Object.\n");
        add_constraints(trans.data);
        objects.push_back(trans);
    }
    vector<int> try_cnt(norm_data.size(), 0);
    it = norm_data.begin();
    while (it != norm_data.end()) {
        if (print_info)
            printf("  Info: Trying to add Normal Object %d...\n", (int)(it - norm_data.begin() + 1));
        ObjectTransformer trans(*it);
        trans.random_scale();
        trans.random_rotate();
        bool success = false;
        for (int ntry = 1; ntry <= MAX_TRY; ntry++) {
            trans.random_move(min_r, max_r);
            if (check_constraints(trans.data)) {
                success = true;
                break;
            }
        }
        if (success) {
            if (print_info)
                printf("    SUCC: Move on to next Object.\n");
            try_cnt[it - norm_data.begin()]++;
            objects.push_back(trans);
            add_constraints(trans.data);
            it++;
        }
        else {
            if (print_info)
                printf("    FAIL: Retrace.\n");
            if (it == norm_data.begin()) {
                cerr << "Error: Failed to join Object!" << endl;
                exit(1);
            }
            try_cnt[it - norm_data.begin()] = 0;
            it--;
            del_constraints(objects.back().data);
            objects.pop_back();
            if (try_cnt[it - norm_data.begin()] >= MAX_TRY) {
                cerr << "Error: Failed to join Object!" << endl;
                exit(1);
            }
        }
    }
    vector<ObjectTransformer>::iterator ito;
    for (ito = objects.begin(); ito != objects.end(); ito++) {
        ito->random_noise();
        sort(ito->data.begin(), ito->data.end(), comp);
        joint_data.insert(joint_data.end(), ito->data.begin(), ito->data.end());
    }
    for (ito = objects.begin(); ito != objects.end(); ito++)
        joint_data.insert(joint_data.end(), ito->noise.begin(), ito->noise.end());
}

bool ObjectJoiner::center_available(TriplePair<float> & newcenter, float newradius) {
    double viewangle_rad = (double)ANGLE_WIDTH / 180 * M_PI;
    double tan_angle = tan(viewangle_rad / 2);
    double dist_x1 = (newcenter.x - tan_angle * newcenter.z) / sqrt(1 + square(tan_angle));
    double dist_y1 = (newcenter.y - tan_angle * newcenter.z) / sqrt(1 + square(tan_angle));
    double dist_x2 = (-newcenter.x - tan_angle * newcenter.z) / sqrt(1 + square(tan_angle));
    double dist_y2 = (-newcenter.y - tan_angle * newcenter.z) / sqrt(1 + square(tan_angle));
    if (dist_x1 < newradius || dist_y1 < newradius || dist_x2 < newradius || dist_y2 < newradius)
        return false;
    vector<ObjectTransformer>::iterator it;
    for (it = objects.begin(); it != objects.end(); it++) {
        float dist = sqrt(
            square(it->center.x - newcenter.x) + 
            square(it->center.y - newcenter.y) + 
            square(it->center.z - newcenter.z)
        );
        if (dist < it->radius + newradius)
            return false;
    }
    return true;
}

bool ObjectJoiner::check_constraints(vector<Point> & points) {
    double viewangle_rad = (double)ANGLE_WIDTH / 180 * M_PI;
    double angle_start = -sin(viewangle_rad / 2);
    double x_angle_step = (2 * sin(viewangle_rad / 2) / (OUTPUT_DIM_X - 1));
    double y_angle_step = (2 * sin(viewangle_rad / 2) / (OUTPUT_DIM_Y - 1));
    vector<Point>::iterator it;
    for (it = points.begin(); it != points.end(); it++) {
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++)
                for (int dz = -1; dz <= 1; dz++) {
                    int offset = get_offset(it->x + dx * coll_dist, it->y + dy * coll_dist, it->z + dz * coll_dist);
                    if (offset == -1 || grid[offset])
                        return false;
                }
        it->r = sqrt(square(it->x) + square(it->y) + square(it->z));
        double x_angle = (it->x / it->r) + (x_angle_step / 2);
        double y_angle = (it->y / it->r) + (y_angle_step / 2);
        int x_index = round((x_angle - angle_start) / x_angle_step);
        int y_index = round((y_angle - angle_start) / y_angle_step);
        if (x_index < 0 || x_index >= OUTPUT_DIM_X || y_index < 0 || y_index >= OUTPUT_DIM_Y)
            return false;
    }
    return true;
}

void ObjectJoiner::add_constraints(vector<Point> & points) {
    vector<Point>::iterator it;
    for (it = points.begin(); it != points.end(); it++) {
        int offset = get_offset(it->x, it->y, it->z);
        if (offset == -1) {
            cerr << "Error: Array index out of range!" << endl;
            exit(1);
        }
        else
            grid[offset] = true;
    }
}

void ObjectJoiner::del_constraints(vector<Point> & points) {
    vector<Point>::iterator it;
    for (it = points.begin(); it != points.end(); it++) {
        int offset = get_offset(it->x, it->y, it->z);
        if (offset == -1) {
            cerr << "Error: Array index out of range!" << endl;
            exit(1);
        }
        else
            grid[offset] = false;
    }
}

int ObjectJoiner::get_offset(float x, float y, float z) {
    int offset = 0;
    int x_pos = round(x / coll_dist);
    int y_pos = round(y / coll_dist);
    int z_pos = round(z / coll_dist);
    if (x_pos < x_min || x_pos > x_max || 
            y_pos < y_min || y_pos > y_max || 
            z_pos < z_min || z_pos > z_max)
        return -1;
    offset += (z_pos - z_min);
    offset += (y_pos - y_min) * z_len;
    offset += (x_pos - x_min) * y_len * z_len;
    return offset;
}