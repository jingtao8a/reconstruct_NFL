//
// Created by chengang on 24-11-21.
//

#include "test1.h"
#include <string>
#include <iostream>

int get_num_keys(std::string name) {
    int tot_num = 0;
    for (int i = name.size() - 1; i >= 0; -- i) {
        if (name[i] == 'M') {
            int p = 1;
            for (int j = i - 1; j >= 0 && name[j] != '-'; j --, p *= 10) {
                tot_num = tot_num + (name[j] - '0') * p;
            }
            tot_num = tot_num * 1000000;
            break;
        }
    }
    return tot_num;
}

int main() {
    std::string path("/data/ycsb-200M.bin.data");
    int start;
    if (path.rfind('/') == std::string::npos) {
        start = 0;
    } else {
        start = path.rfind('/') + 1;
    }
    int end = path.rfind(".bin.data");
    int length = end - start;
    auto data_name = path.substr(start, length);
    std::cout << data_name << std::endl;
    std::cout << get_num_keys(path) << std::endl;
    return 0;
}