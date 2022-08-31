#include "objwriter.h"

OriginPoint::OriginPoint(double _r, double _refl) {
    r = _r;
    refl = _refl;
}

bool OriginPoint::operator<(const OriginPoint & p) const {
    return refl > p.refl;
}

ObjectWriter::ObjectWriter(vector<ObjectPoint> & _rawdata, bool rm_near): rawdata(_rawdata) {
    double viewangle_rad = (double)ANGLE_WIDTH / 180 * M_PI;
    double angle_start = -sin(viewangle_rad / 2);
    double x_angle_step = (2 * sin(viewangle_rad / 2) / (OUTPUT_DIM_X - 1));
    double y_angle_step = (2 * sin(viewangle_rad / 2) / (OUTPUT_DIM_Y - 1));
    vector<ObjectPoint>::iterator it;
    if (!rm_near) {
        for (it = rawdata.begin(); it != rawdata.end(); it++) {
            double x_angle = (it->x / it->r) + (x_angle_step / 2);
            double y_angle = (it->y / it->r) + (y_angle_step / 2);
            int x_index = round((x_angle - angle_start) / x_angle_step);
            int y_index = round((y_angle - angle_start) / y_angle_step);
            if (x_index >= 0 && x_index < OUTPUT_DIM_X && y_index >= 0 && y_index < OUTPUT_DIM_Y)
                data[x_index][y_index].push_back(OriginPoint(it->r, it->refl));
        }
    }
    else {
        set<double, DistComp> diff_dist[OUTPUT_DIM_X][OUTPUT_DIM_Y];
        for (it = rawdata.begin(); it != rawdata.end(); it++) {
            double x_angle = (it->x / it->r) + (x_angle_step / 2);
            double y_angle = (it->y / it->r) + (y_angle_step / 2);
            int x_index = round((x_angle - angle_start) / x_angle_step);
            int y_index = round((y_angle - angle_start) / y_angle_step);
            if (x_index >= 0 && x_index < OUTPUT_DIM_X && y_index >= 0 && y_index < OUTPUT_DIM_Y) {
                if (diff_dist[x_index][y_index].find(it->r) == diff_dist[x_index][y_index].end()) {
                    diff_dist[x_index][y_index].insert(it->r);
                    data[x_index][y_index].push_back(OriginPoint(it->r, it->refl));
                }
            }
        }
    }
}

void ObjectWriter::insert_object(const ObjectWriter & obj) {
    for (int i = 0; i < OUTPUT_DIM_X; i++)
        for (int j = 0; j < OUTPUT_DIM_Y; j++)
            data[i][j].insert(data[i][j].end(), obj.data[i][j].begin(), obj.data[i][j].end());
}

void ObjectWriter::write_object_to_mat(string outdir) {
    MATFile * p_file = matOpen(outdir.c_str(), "w");
    if (!p_file) {
        cerr << "Error: Unable to open file " << outdir << "!" << endl;
        exit(1);
    }
    const mwSize dims[3] = {OUTPUT_DIM_X, OUTPUT_DIM_Y, OUTPUT_DIM_N};
	mxArray * p_array_ai = mxCreateNumericArray(3, dims, mxDOUBLE_CLASS, mxREAL);
    if (!p_array_ai) {
        cerr << "Error: Unable to create mxArray for variable ai!" << endl;
        exit(1);
    }
	mxArray * p_array_zi = mxCreateNumericArray(3, dims, mxDOUBLE_CLASS, mxREAL);
    if (!p_array_zi) {
        cerr << "Error: Unable to create mxArray for variable zi!" << endl;
        exit(1);
    }
	double * p_mat_ai = (double *)mxGetPr(p_array_ai);
    double * p_mat_zi = (double *)mxGetPr(p_array_zi);
    for (int i = 0; i < OUTPUT_DIM_X; i++)
        for (int j = 0; j < OUTPUT_DIM_Y; j++) {
            sort(data[i][j].begin(), data[i][j].end());
            int len = min((int)data[i][j].size(), OUTPUT_DIM_N);
            for (int k = 0; k < len; k++) {
                int index = k * OUTPUT_DIM_Y * OUTPUT_DIM_X + j * OUTPUT_DIM_X + i;
                p_mat_ai[index] = data[i][j][k].refl;
                p_mat_zi[index] = data[i][j][k].r;
            }
        }
    const char * fields[2] = {"ai", "zi"};
    mxArray * p_array = mxCreateStructMatrix(1, 1, 2, fields);
    if (!p_array) {
        cerr << "Error: Unable to create mxArray for variable image_result_2D!" << endl;
        exit(1);
    }
    mxSetField(p_array, 0, "ai", p_array_ai);
    mxSetField(p_array, 0, "zi", p_array_zi);
    matPutVariable(p_file, "image_result_2D", p_array);
    matClose(p_file);
    printf("Sonar data saved to %s.\n", outdir.c_str());
}

void ObjectWriter::write_object_to_txt(string outdir) {
    ofstream outfile(outdir.c_str());
    if (!outfile.is_open()) {
        cerr << "Error: Unable to open file " << outdir << "!" << endl;
        exit(1);
    }
    vector<ObjectPoint>::iterator it;
    for (it = rawdata.begin(); it != rawdata.end(); it++) {
        char buffer[50];
        sprintf(buffer, "%.3f %.3f %.3f %.3f", it->x, it->y, it->z, it->refl);
        outfile << buffer << endl;
    }
    outfile.close();
    printf("Sonar data saved to %s.\n", outdir.c_str());
}