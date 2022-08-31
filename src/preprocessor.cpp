#include "preprocessor.h"

DataPreprocessor::DataPreprocessor(vector<Point> & d): data(d) {
    color_max = color_min = 0;
}

void DataPreprocessor::load_mat(const char * filenm, double thre, bool enhance) {
    MATFile * p_file = matOpen(filenm, "r");
    if (!p_file) {
        cerr << "Error: Unable to open file " << filenm << "!" << endl;
        exit(1);
    }
    mxArray * p_array = matGetVariable(p_file, "image_result_2D");
    if (!p_array) {
        cerr << "Error: Unable to load variable image_result_2D!" << endl;
        exit(1);
    }
    mxArray * p_array_ai = mxGetField(p_array, 0, "ai");
    if (!p_array_ai) {
        cerr << "Error: Unable to load variable ai!" << endl;
        exit(1);
    }
    mxArray * p_array_zi = mxGetField(p_array, 0, "zi");
    if (!p_array_zi) {
        cerr << "Error: Unable to load variable zi!" << endl;
        exit(1);
    }
    if (!enhance)
        cerr << "Warning: Metrics will be inprecise due to --noenhance option!" << endl;
    double viewangle_rad = (double)ANGLE_WIDTH / 180 * M_PI;
    const size_t * shape = mxGetDimensions(p_array_ai);
    int dim1 = shape[0];
    int dim2 = shape[1];
    int dim3 = shape[2];
    double * p_mat_ai = (double *)mxGetPr(p_array_ai);
    double * p_mat_zi = (double *)mxGetPr(p_array_zi);
    double angle_start = -sin(viewangle_rad / 2);
    double x_angle_step = (2 * sin(viewangle_rad / 2) / (dim1 - 1));
    double y_angle_step = (2 * sin(viewangle_rad / 2) / (dim2 - 1));
    int cnt = 0;
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            for (int k = 0; k < dim3; k++) {
                int index = k * dim2 * dim1 + j * dim1 + i;
                double intensity = p_mat_ai[index];
                double r = p_mat_zi[index];
                if (intensity < thre)
                    continue;
                double x_angle, y_angle;
                if (enhance) {
                    int n_points = floor(r / POINT_RANGE) + 1;
                    double x_step = x_angle_step / n_points;
                    double y_step = y_angle_step / n_points;
                    x_angle = angle_start + i * x_angle_step - ((n_points - 1) * x_step / 2);
                    y_angle = angle_start + j * y_angle_step - ((n_points - 1) * y_step / 2);
                    for (int w = 0; w < n_points; w++)
                        for (int h = 0; h < n_points; h++) {
                            double x_convert = r * (x_angle + w * x_step - (x_angle_step / 2));
                            double y_convert = r * (y_angle + h * y_step - (y_angle_step / 2));
                            double z_convert = sqrt(square(r) - square(x_convert) - square(y_convert));
                            if (n_points % 2 == 0 || w != n_points / 2 || h != n_points / 2)
                                data.push_back(Point(x_convert, y_convert, -z_convert, intensity, r, false));
                        }
                }
                x_angle = angle_start + i * x_angle_step;
                y_angle = angle_start + j * y_angle_step;
                double x_convert = r * (x_angle - (x_angle_step / 2));
                double y_convert = r * (y_angle - (y_angle_step / 2));
                double z_convert = sqrt(square(r) - square(x_convert) - square(y_convert));
                data.push_back(Point(x_convert, y_convert, -z_convert, intensity, r, true));
            }
        }
    }
    matClose(p_file);
    sort(data.begin(), data.end());
    if (data.size() > 0) {
        color_min = data.begin()->color;
        color_max = data.rbegin()->color;
    }
    else {
        cerr << "Error: Data length is zero!" << endl;
        exit(1);
    }
}

void DataPreprocessor::load_txt(const char * filenm, double thre) {
    ifstream infile(filenm);
    if (!infile.is_open()) {
        cerr << "Error: Unable to open file " << filenm << "!" << endl;
        exit(1);
    }
    cerr << "Warning: Resolution enhancement is disabled under .txt input mode!" << endl;
    GLfloat x, y, z, color;
    while (infile >> x) {
        infile >> y;
        infile >> z;      
        infile >> color;
        GLfloat r = sqrt(square(x) + square(y) + square(z));
        if (color > thre)
            data.push_back(Point(x, y, z, color, r, true));
    }
    infile.close();
    sort(data.begin(), data.end());
    if (data.size() > 0) {
        color_min = data.begin()->color;
        color_max = data.rbegin()->color;
    }
    else {
        cerr << "Error: Data length is zero!" << endl;
        exit(1);
    }
}

NoiseFilter::NoiseFilter(int _dist, int _thre) {
    dist = _dist;
    thre = _thre;
    point_cnt = vector<int>(((NRANGE * K) + dist - 1) / dist, 0);
}

void NoiseFilter::set_filter(vector<Point> & v) {
    vector<Point>::iterator it;
    for (it = v.begin(); it != v.end(); it++) {
        if (!it->is_origin)
            continue;
        int index = floor(it->r / dist);
        if (index < 0)
            index = 0;
        else if (index > point_cnt.size() - 1)
            index = point_cnt.size() - 1;
        point_cnt[index] += 1;
    }
}

bool NoiseFilter::is_noise(Point & p) {
    int index = floor(p.r / dist);
    if (index < 0)
        index = 0;
    else if (index > point_cnt.size() - 1)
        index = point_cnt.size() - 1;
    return (point_cnt[index] >= thre);
}

void NoiseFilter::process_data(vector<Point> & raw, vector<Point> & denoised) {
    vector<Point>::iterator it;
    for (it = raw.begin(); it != raw.end(); it++) {
        int index = floor(it->r / dist);
        if (index < 0)
            index = 0;
        else if (index > point_cnt.size() - 1)
            index = point_cnt.size() - 1;
        if (point_cnt[index] < thre)
            denoised.push_back(*it);
    }
}