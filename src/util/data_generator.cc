#include "workload.h"
#include "util/common.h"
#include "util/zipf.h"

using namespace nfl;

/***
# Configurations for synthetic dataset
mean=0
var_list=(1 2 4 8)
begin=0
end=2147483648 # 2^31

gen (data directory) lognormal (number millions of keys) (mean) (variance)

gen (data directory) uniform (number millions of keys) (begin) (end)
***/
int main(int argc, char* argv[]) {
    if (argc < 6) {
      std::cout << "No enough parameters for generating dataset" << std::endl;
      std::cout << "Please input: gen dataset (data directory)" 
                << "(distribution name) (number millions of keys) " << std::endl;
      std::cout << "for example: \n"
                   "gen (data directory) lognormal (number millions of keys) (mean) (variance)\n"
                   "gen (data directory) uniform (number millions of keys) (begin) (end)" << std::endl;
      exit(-1);
    }
    std::vector<double> keys;
    std::string data_dir = std::string(argv[1]);
    std::string distribution_name = std::string(argv[2]);
    int num_keys = std::stoi(argv[3]) * 1000000;
    if (distribution_name == "lognormal") {
      double mean = ston<char*, double>(argv[4]);
      double var = ston<char*, double>(argv[5]);
      std::lognormal_distribution<double> dist(mean, var);
      std::string workload_path = path_join(data_dir, distribution_name + "_" +
                                  str<int>(num_keys / 1000000) + "M_double_var(" +
                                  str<double>(var) + ").bin");
      generate_synthetic_keys<std::lognormal_distribution<double>, double>(
                                        dist, num_keys, keys, workload_path);
    } else if (distribution_name == "uniform") {
      double begin = ston<char*, double>(argv[4]);
      double end = ston<char*, double>(argv[5]);
      std::uniform_real_distribution<double> dist(begin, end);
      std::string workload_path = path_join(data_dir, distribution_name + "_" +
                                  str<int>(num_keys / 1000000) + "M_double.bin");
      generate_synthetic_keys<std::uniform_real_distribution<double>, double>(
                            dist, num_keys, keys, workload_path);
    } else {
      std::cout << "Unsupported distribution name [" << distribution_name << "]"
                << std::endl;
      exit(-1);
    }
  return 0;
}