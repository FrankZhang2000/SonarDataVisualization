#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unistd.h>
#include <getopt.h>
#include "preprocessor.h"
#include "classifier.h"
#include "utils.h"
#include "settings.h"

using namespace std;

enum arg_types {
    arg_config, arg_out, arg_info, arg_help
};

void print_usage() {
    cout << "OVERVIEW: a simple program to calculate and collect metrics" << endl;
    cout << "USAGE: getmetrics [options]" << endl;
    cout << "OPTIONS: " << endl;
    cout << "  --help/-h\t\t\tPrint help message" << endl;
    cout << "  --config/-c <file_name>\tUse file <file_name> as config file" << endl;
    cout << "  --out/-o <file_name>\t\tUse file <file_name> as output file" << endl;
    cout << "  --showinfo\t\t\tPrint detailed info while running" << endl;
}

int main(int argc, char ** argv) {
    string configfilenm, outfilenm;
    bool has_configfile = false;
    bool has_outfile = false;
    bool print_info = false;
    static struct option long_options[] = {
        {"config", required_argument, NULL, (int)arg_types::arg_config}, 
        {"out", required_argument, NULL, (int)arg_types::arg_out},
        {"showinfo", no_argument, NULL, (int)arg_types::arg_info},
        {"help", no_argument, NULL, (int)arg_types::arg_help}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "c:o:h", long_options, NULL)) != EOF) {
        switch (opt) {
        case 'c':
        case (int)arg_types::arg_config:
            if (!optarg) {
                cerr << "Error: Config file is not correctly specified!" << endl;
                print_usage();
                exit(1);
            }
            configfilenm = optarg;
            has_configfile = true;
            break;
        case 'o':
        case (int)arg_types::arg_out:
            if (!optarg) {
                cerr << "Error: Output file is not correctly specified!" << endl;
                print_usage();
                exit(1);
            }
            outfilenm = optarg;
            has_outfile = true;
            break;
        case 'h':
        case (int)arg_types::arg_help:
            print_usage();
            exit(1);
            break;
        case (int)arg_types::arg_info:
            print_info = true;
            break;
        default:
            cerr << "Error: Wrong argument format!" << endl;
            print_usage();
            exit(1);
            break;
        }
    }
    if (!has_configfile) {
        cerr << "Error: Config file is not correctly specified!" << endl;
        print_usage();
        exit(1);
    }
    else if (!has_outfile) {
        cerr << "Error: Output file is not correctly specified!" << endl;
        print_usage();
        exit(1);
    }
    ifstream configfile(configfilenm);
    if (!configfile.is_open()) {
        cerr << "Error: Unable to open config file " << configfilenm << "!" << endl;
        exit(1);
    }
    vector<string> outdata;
    string filenm;
    double thre, dist;
    int pointcnt, noisethre;
    int data_cnt = 0;
    while (configfile >> filenm) {
        if (print_info)
            printf("Processing input file %s:\n", filenm.c_str());
        configfile >> thre >> dist >> pointcnt >> noisethre;
        vector<Point> points;
        DataPreprocessor preprocessor(points);
        preprocessor.load_mat(filenm.c_str(), thre, true);
        if (points.size() == 0) {
            cerr << "Error: Data length of file is zero!" << endl;
            exit(1);
        }
        NoiseFilter filter(FILTER_DIST, noisethre);
        filter.set_filter(points);
        vector<Point>::iterator it;
        for (it = points.begin(); it != points.end(); it++) {
            if (filter.is_noise(*it))
                it->group_id = -1;
        }
        Classifier classifier(dist, pointcnt);
        int group_count = classifier.group_data(points);
        int group_id;
        while (configfile >> group_id) {
            if (group_id == -1)
                break;
            else if (group_id <= 0) {                    
                if (print_info)
                    printf("  Group ID %d is not positive, skip\n", group_id);
            }
            else if (group_id > group_count) {
                if (print_info)
                    printf("  Group ID %d exceeds number of Group %d, skip\n", group_id, group_count);
            }
            else {
                if (print_info)
                    printf("  Calculating metrics of Group %d...\n", group_id);
                classifier.calc_groupinfo(group_id - 1);
                Group & g = classifier.group_list[group_id - 1];
                char buf[150];
                sprintf(
                    buf, 
                    "%d,%s,%d,%d,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f", 
                    ++data_cnt, filenm.c_str(), group_id, g.point_count, g.edge_pointcnt, 
                    g.length, g.max_width, g.max_height, g.bulk, 
                    g.approx_length, g.edge_length, g.act_width, g.act_height, 
                    g.avg_refl, g.min_refl, g.max_refl, g.dev_refl
                );
                outdata.push_back(buf);
            }
        }
    }
    configfile.close();
    ofstream outfile(outfilenm);
    if (!outfile.is_open()) {
        cerr << "Error: Unable to open file " << outfilenm << "!" << endl;
        exit(1);
    }
    outfile << "ID,FileName,GroupID,TotPointCnt,EdgePointCnt,";
    outfile << "Max.Length,Max.Width,Max.Height,Bulk,";
    outfile << "Act.Length.repr,Act.Length.cent,Act.Width,Act.Height,";
    outfile << "Avg.Refl,Min.Refl,Max.Refl,Std.Dev.Refl" << endl;
    vector<string>::iterator it;
    for (it = outdata.begin(); it != outdata.end(); it++)
        outfile << *it << endl;
    outfile.close();
    printf("Metrics of all Groups saved to file %s.\n", outfilenm.c_str());
    return 0;
}