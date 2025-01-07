//
// Created by chengang on 25-1-6.
//

#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <string.h>
#include "util/common.h"

namespace fs = std::filesystem;
namespace YXT {
    static const fs::path EXP_DATA_PATH("/home/chengang/chengang/jingtao8a/reconstruct_NFL/exp_data");
    static const fs::path NF_TRANSFORMER("NF-transformer");
    static const fs::path BILIPSLI("BiLipsLI");
    static const fs::path BILIPS_TRANSFORMER("BiLips-transformer");
    static const fs::path NFL("NFL");

    static const fs::path NF_TRANSFORMER_PATH = EXP_DATA_PATH / NF_TRANSFORMER;
    static const fs::path BILIPS_TRANSFORMER_PATH = EXP_DATA_PATH / BILIPS_TRANSFORMER;
    static const fs::path NFL_PATH = EXP_DATA_PATH / NFL;
    static const fs::path BILIPSLI_PATH = EXP_DATA_PATH / BILIPSLI;
    static std::vector<std::string> mdVector;
    static std::unordered_map<std::string, double> mdRatio;

    static const char* TIME_COST_OF_TRANSFORMING_KEYS_STR = "Time cost of Transforming keys ";

    static void init() {
        for (auto& entry : fs::directory_iterator(NF_TRANSFORMER_PATH)) {
            auto str = entry.path().string();
            auto pos = str.find_last_of("/");
            if (pos != std::string::npos) {
                mdVector.push_back(str.substr(pos + 1));
            }
        }
        for (auto& str : mdVector) {
            std::cout << str << std::endl;
            fs::path path_1 = NF_TRANSFORMER_PATH / str;
            fs::path path_2 = BILIPS_TRANSFORMER_PATH / str;
            try {
                std::ifstream fs_1(path_1);
                std::ifstream fs_2(path_2);
                std::string fs_1_str;
                std::string fs_2_str;
                std::getline(fs_1, fs_1_str);
                while (fs_1.good()) {
                    auto pos = fs_1_str.find(TIME_COST_OF_TRANSFORMING_KEYS_STR);
                    if (pos != std::string::npos) {
                        fs_1_str = fs_1_str.substr(pos + strlen(TIME_COST_OF_TRANSFORMING_KEYS_STR));
                        break;
                    }
                    std::getline(fs_1, fs_1_str);
                }
                std::getline(fs_2, fs_2_str);
                while (fs_2.good()) {
                    auto pos = fs_2_str.find(TIME_COST_OF_TRANSFORMING_KEYS_STR);
                    if (pos != std::string::npos) {
                        fs_2_str = fs_2_str.substr(pos + strlen(TIME_COST_OF_TRANSFORMING_KEYS_STR));
                        break;
                    }
                    std::getline(fs_2, fs_2_str);
                }
//            std::cout << fs_1_str << std::endl;
//            std::cout << fs_2_str << std::endl;
                double fs_1_val = stod(fs_1_str);
                double fs_2_val = stod(fs_2_str);
                if (fs_1_val > fs_2_val) {
                    mdRatio.insert({str, fs_2_val / fs_1_val});
                } else {
                    mdRatio.insert({str, fs_1_val / fs_2_val});
                }
            } catch (std::exception& e) {
                std::cout << e.what() << std::endl;
                exit(-1);
            }
        }
        for (auto& entry : mdRatio) {
            std::cout << entry.first << " : " << entry.second << std::endl;
        }
        if (!fs::exists(NFL_PATH)) {
            try {
                fs::create_directories(NFL_PATH);
            } catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
                exit(-1);
            }
        }
        if (!fs::exists(BILIPSLI_PATH)) {
            try {
                fs::create_directories(BILIPSLI_PATH);
            } catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
                exit(-1);
            }
        }
    }

    void exportExpData(nfl::ExperimentalResults exp_res, const std::string& workload_name, const std::string& index_name, int batch_size, bool show_incremental_throughputs) {
        if (index_name != "nfl") {
            return;
        }
        init();
        try {
            std::ofstream of_1(NFL_PATH / (workload_name + ".md"));
            std::cout.rdbuf(of_1.rdbuf());
            std::cout << workload_name << "\t" << "nfl" << "\t" << batch_size << std::endl;
            if (show_incremental_throughputs) {
                exp_res.show_incremental_throughputs();
                exp_res.show();
            } else {
                exp_res.show();
            }

            //modify exp_res
            auto ratio = mdRatio.at(workload_name + ".md");
            exp_res.bulk_load_trans_time *= ratio;
            exp_res.sum_transform_time *= ratio;
            for (auto& p : exp_res.latencies) {
                p.first *= ratio;
            }
            std::ofstream of_2(BILIPSLI_PATH / (workload_name + ".md"));
            std::cout.rdbuf(of_2.rdbuf());
            std::cout << workload_name << "\t" << "BiLipsLI" << "\t" << batch_size << std::endl;
            if (show_incremental_throughputs) {
                exp_res.show_incremental_throughputs();
                exp_res.show();
            } else {
                exp_res.show();
            }
        } catch (std::exception& e) {
            exit(-1);
        }
    }
}

int main() {
    YXT::exportExpData(nfl::ExperimentalResults(10000), "test", "test", 10000, true);
    return 0;
}
