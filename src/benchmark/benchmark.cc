#include "benchmark/benchmark.h"

using namespace nfl;

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "No enough parameters" << std::endl;
    std::cout << "Please input: benchmark (index name) "
              << "(workload path) [config path] "
              << "[show incremental updates]" << std::endl;
    exit(-1);
  }
  std::string index_name = std::string(argv[1]);
  std::string workload_path = std::string(argv[2]);
  std::string config_path = argc > 3 ? std::string(argv[3]) : "";
  std::string show_inc_thro = argc > 4 ? std::string(argv[4]) : "";
  std::string workload_name = get_workload_name(workload_path);
  auto index = workload_name.rfind("_");
  std::string batch_size_str = workload_name.substr(index + 1, workload_name.length() - index - 1);
  int batch_size = std::stoi(batch_size_str);
  srand(kSEED);
  Benchmark<double, long long> benchmark;
  benchmark.run_workload(index_name, batch_size, workload_path, config_path, show_inc_thro != "");
  return 0;
}
