#include "benchmark/benchmark.h"

using namespace nfl;

int main(int argc, char* argv[]) {
  if (argc < 4) {
    std::cout << "No enough parameters" << std::endl;
    std::cout << "Please input: benchmark (index name) (batch size) "
              << "(workload path) [config path] "
              << "[show incremental updates]" << std::endl;
    exit(-1);
  }
  std::string index_name = std::string(argv[1]);
  int batch_size = std::stoi(argv[2]);
  std::string workload_path = std::string(argv[3]);
  std::string config_path = argc > 4 ? std::string(argv[4]) : "";
  std::string show_inc_thro = argc > 5 ? std::string(argv[5]) : "";
  srand(kSEED);
  Benchmark<double, long long> benchmark;
  benchmark.run_workload(index_name, batch_size, workload_path, config_path, show_inc_thro != "");
  return 0;
}
