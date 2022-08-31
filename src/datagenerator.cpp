#include <iostream>
#include <cmath>
#include <cstring>
#include <random>
#include <unistd.h>
#include <getopt.h>
#include "joinobject.h"
#include "preprocessor.h"
#include "objwriter.h"
#include "settings.h"
#include "utils.h"

using namespace std;

enum arg_types {
    arg_norm, arg_fixed, arg_out, arg_help, arg_num, 
    arg_name, arg_info, arg_mindist, arg_maxdist, arg_config, arg_objdist
};

int read_config(string configfilenm, vector<string> & norm, vector<string> & fixed) {
    ifstream configfile(configfilenm);
    if (!configfile.is_open()) {
        cerr << "Error: Unable to open config file " << configfilenm << "!" << endl;
        exit(1);
    }
    int cnt = 0;
    string filenm;
    while (configfile >> filenm) {
        string type;
        int num;
        configfile >> type >> num;
        if (num <= 0) {
            fprintf(stderr, "Error: File %s has invalid object count %d, skip\n", 
                filenm.c_str(), num);
        }
        else {
            if (type == "norm") {
                norm.push_back(filenm);
                for (int i = 0; i < num - 1; i++)
                    norm.push_back("::");
                cnt += num;
            }
            else if (type == "fixed") {
                fixed.push_back(filenm);
                for (int i = 0; i < num - 1; i++)
                    fixed.push_back("::");
                cnt += num;
            }
            else
                fprintf(stderr, "Error: File %s has undefined object type %s, skip\n", 
                    filenm.c_str(), type.c_str());
        }
    }
    configfile.close();
    return cnt;
}

void print_usage() {
    cout << "OVERVIEW: a simple underwater sonar data generator" << endl;
    cout << "USAGE: datagenerator [options]" << endl;
    cout << "OPTIONS: " << endl;
    cout << "  --help/-h\t\t\tPrint help message" << endl;
    cout << "  --showinfo\t\t\tPrint detailed information while running" << endl;
    cout << "  --norm <file_name>\t\tUse file <file_name> as input for normal object" << endl;
    cout << "  --fixed <file_name>\t\tUse file <file_name> as input for fixed object" << endl;
    cout << "  --config/-c <file_name>\tUse file <file_name> as config file" << endl;
    cout << "  --mindist <dist>\t\tSet minimum placing distance of normal object to <dist>" << endl;
    cout << "  --maxdist <dist>\t\tSet maximum placing distance of normal object to <dist>" << endl;
    cout << "  --objdist <dist>\t\tSet minimum distance between objects to <dist>" << endl;
    cout << "  --outdir/-o <out_dir>\t\tUse directory <out_dir> for output" << endl;
    cout << "  --num/-n <out_num>\t\tSet number of data generated to <out_num>" << endl;
    cout << "  --name <file_name>\t\tUse file <file_name> as output" << endl;
}

int main(int argc, char ** argv) {
    random_device rd;
    srand(rd());
    double color_thre = 0;
    int out_num = 1;
    bool print_info = false;
    vector<string> norm_infiles;
    vector<string> fixed_infiles;
    string outdir;
    string name = "joint_result_2D";
    bool has_infile = false;
    bool has_outdir = false;
    float min_dist = R_DOWNTHRE;
    float max_dist = R_UPTHRE;
    float coll_dist = COLL_DIST_INIT;
    static struct option long_options[] = {
        {"norm", required_argument, NULL, (int)arg_types::arg_norm}, 
        {"fixed", required_argument, NULL, (int)arg_types::arg_fixed}, 
        {"outdir", required_argument, NULL, (int)arg_types::arg_out}, 
        {"help", no_argument, NULL, (int)arg_types::arg_help},
        {"num", required_argument, NULL, (int)arg_types::arg_num},
        {"name", required_argument, NULL, (int)arg_types::arg_name},
        {"showinfo", no_argument, NULL, (int)arg_types::arg_info},
        {"mindist", required_argument, NULL, (int)arg_types::arg_mindist},
        {"maxdist", required_argument, NULL, (int)arg_types::arg_maxdist},
        {"objdist", required_argument, NULL, (int)arg_types::arg_objdist},
        {"config", required_argument, NULL, (int)arg_types::arg_config}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "ho:n:c:", long_options, NULL)) != EOF) {
        switch (opt) {
        case (int)arg_types::arg_norm:
            if (!optarg) {
                cerr << "Error: Input file is not correctly specified!" << endl;
                print_usage();
                exit(1);
            }
            norm_infiles.push_back(optarg);
            has_infile = true;
            break;
        case (int)arg_types::arg_fixed:
            if (!optarg) {
                cerr << "Error: Input file is not correctly specified!" << endl;
                print_usage();
                exit(1);
            }
            fixed_infiles.push_back(optarg);
            has_infile = true;
            break;
        case 'c':
        case (int)arg_types::arg_config:
            if (!optarg) {
                cerr << "Error: Config file is not correctly specified!" << endl;
                print_usage();
                exit(1);
            }
            if (read_config(optarg, norm_infiles, fixed_infiles) > 0)
                has_infile = true;
            break;
        case 'o':
        case (int)arg_types::arg_out:
            if (!optarg) {
                cerr << "Error: Output directory is not correctly specified!" << endl;
                print_usage();
                exit(1);
            }
            outdir = optarg;
            has_outdir = true;
            break;
        case 'h':
        case (int)arg_types::arg_help:
            print_usage();
            exit(1);
            break;
        case 'n':
        case (int)arg_types::arg_num:
            out_num = stoi(optarg);
            break;
        case (int)arg_types::arg_name:
            name = optarg;
            break;
        case (int)arg_types::arg_info:
            print_info = true;
            break;
        case (int)arg_types::arg_mindist:
            min_dist = stof(optarg);
            if (min_dist < R_DOWNTHRE) {
                printf("Info: Minimum placing distance %.3f exceeds lower limit %.3f, it is set to %.3f.\n", 
                    min_dist, (float)R_DOWNTHRE, (float)R_DOWNTHRE);
                min_dist = R_DOWNTHRE;
            }
            else if (min_dist > R_UPTHRE) {
                printf("Info: Minimum placing distance %.3f exceeds upper limit %.3f, it is set to %.3f.\n", 
                    min_dist, (float)R_UPTHRE, (float)R_UPTHRE);
                min_dist = R_UPTHRE;
            }
            break;
        case (int)arg_types::arg_maxdist:
            max_dist = stof(optarg);
            if (max_dist > R_UPTHRE) {
                printf("Info: Maximum placing distance %.3f exceeds upper limit %.3f, it is set to %.3f.\n", 
                    max_dist, (float)R_UPTHRE, (float)R_UPTHRE);
                max_dist = R_UPTHRE;
            }
            else if (max_dist < R_DOWNTHRE) {
                printf("Info: Maximum placing distance %.3f exceeds lower limit %.3f, it is set to %.3f.\n", 
                    max_dist, (float)R_DOWNTHRE, (float)R_DOWNTHRE);
                max_dist = R_DOWNTHRE;
            }
            break;
        case (int)arg_types::arg_objdist:
            coll_dist = stof(optarg);
            if (coll_dist > COLL_DIST_MAX) {
                printf("Info: Minimum distance between objects %.3f exceeds upper limit %.3f, it is set to %.3f.\n", 
                    coll_dist, (float)COLL_DIST_MAX, (float)COLL_DIST_MAX);
                coll_dist = COLL_DIST_MAX;
            }
            else if (coll_dist < COLL_DIST_MIN) {
                printf("Info: Minimum distance between objects %.3f exceeds lower limit %.3f, it is set to %.3f.\n", 
                    coll_dist, (float)COLL_DIST_MIN, (float)COLL_DIST_MIN);
                coll_dist = COLL_DIST_MIN;
            }
            break;
        default:
            cerr << "Error: Wrong argument format!" << endl;
            print_usage();
            exit(1);
            break;
        }
    }
    if (!has_infile) {
        cerr << "Error: Input file is not correctly specified!" << endl;
        print_usage();
        exit(1);
    }
    else if (!has_outdir) {
        cerr << "Error: Output file is not correctly specified!" << endl;
        print_usage();
        exit(1);
    }
    if (min_dist > max_dist) {
        cerr << "Error: Minimum and Maximum placing distances are not correctly specified!" << endl;
        exit(1);
    }
    if (outdir.back() != '/')
        outdir.push_back('/');
    int num_fixed_inputs = fixed_infiles.size();
    vector<vector<Point> > fixed_objects(num_fixed_inputs);
    for (int i = 0; i < num_fixed_inputs; i++) {
        if (fixed_infiles[i] == "::")
            fixed_objects[i] = vector<Point>(fixed_objects[i - 1].begin(), fixed_objects[i - 1].end());
        else {
            DataPreprocessor preprocessor(fixed_objects[i]);
            preprocessor.load_mat(fixed_infiles[i].c_str(), REFL_THRE, true);
        }
    }
    int num_norm_inputs = norm_infiles.size();
    vector<vector<Point> > norm_objects(num_norm_inputs);
    for (int i = 0; i < num_norm_inputs; i++) {
        if (norm_infiles[i] == "::")
            norm_objects[i] = vector<Point>(norm_objects[i - 1].begin(), norm_objects[i - 1].end());
        else {
            DataPreprocessor preprocessor(norm_objects[i]);
            preprocessor.load_mat(norm_infiles[i].c_str(), REFL_THRE, true);
        }
    }
    for (int i = 0; i < out_num; i++) {
        if (print_info)
            printf("Info: Generating data %d...\n", i + 1);
        ObjectJoiner joiner(norm_objects, fixed_objects, print_info, coll_dist);
        joiner.join_objects(min_dist, max_dist);
        if (print_info)
            printf("Info: Data %d generated successfully.\n", i + 1);
        ObjectWriter writer(joiner.joint_data, true);
        char filenm[100];
        sprintf(filenm, "%s%s_%d.mat", outdir.c_str(), name.c_str(), i + 1);
        writer.write_object_to_mat(filenm);
    }
    return 0;
}