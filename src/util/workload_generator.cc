#include "workload.h"
#include "util/common.h"
#include "util/zipf.h"

using namespace nfl;
int get_num_keys(std::string name) {
    int tot_num = 0;
    for (int i = name.size() - 1; i >= 0; -- i) {
        if (name[i] == 'M') {
            int p = 1;
            for (int j = i - 1; j >= 0 && name[j] != '_'; j --, p *= 10) {
                tot_num = tot_num + (name[j] - '0') * p;
            }
            tot_num = tot_num * 1000000;
            break;
        }
    }
    return tot_num;
}

std::string get_name(std::string data_path) {
    int start;
    if (data_path.rfind('/') == std::string::npos) {
        start = 0;
    } else {
        start = data_path.rfind('/') + 1;
    }
    int end = data_path.rfind(".bin");
    int length = end - start;
    auto data_name = data_path.substr(start, length);
    return data_name;
}

template<typename KT, typename VT>
void generate_requests(std::string output_path, std::string data_path, std::string dist_name, int batch_size, double init_frac, double read_frac, double kks_frac) {
  // Load data
  std::string data_name = get_name(data_path);
  int num_keys = get_num_keys(data_name);
  std::vector<std::pair<KT, VT>> kvs;
  load_source_data(data_path, kvs, num_keys);
  std::sort(kvs.begin(), kvs.end(), 
    [](auto const& a, auto const& b) {
      return a.first < b.first;
  });
  int tot_num = kvs.size();
  int init_idx = int(tot_num * init_frac);
  // Verify unique data
  for (int i = 1; i < kvs.size(); ++ i) {
    if (compare(kvs[i].first, kvs[i - 1].first)) {
      std::cout << std::fixed << "Duplicated data" << std::endl
                << i - 1 << "th key [" << kvs[i - 1].first << "]" << std::endl
                << i << "th key [" << kvs[i].first << "]" << std::endl;
      exit(-1);
    }
  }
  // Generate the read-write workloads
  // Prepare the out-of-bound data
  int kks_idx = static_cast<int>(tot_num * kks_frac);
  int oob_num = tot_num - kks_idx;
  std::cout << "Total Number\t[" << tot_num << "]\nInitial Number\t" << init_idx << "]\nKKS Number\t[" << kks_idx << "]\nOOB Number\t[" << oob_num << "]" << std::endl;
  std::vector<std::pair<KT, VT>> oob_data;
  oob_data.reserve(oob_num);
  for (int i = kks_idx; i < tot_num; ++i) {
      oob_data.push_back(kvs[i]);
  }
  // Shuffle the known-key-space data
  if (kks_idx > 2) {
      shuffle(kvs, 1, kks_idx - 1);
      std::swap(kvs[init_idx - 1], kvs[kks_idx - 1]);
  }

  // Prepare the data for bulk loading
  std::vector<std::pair<KT, VT>> existing_data;
  std::vector<Request<KT, VT>> reqs;
  reqs.reserve(tot_num);
  existing_data.reserve(init_idx);
  for (int i = 0; i < init_idx; ++ i) {
    reqs.push_back({kBulkLoad, kvs[i]});
    existing_data.push_back(kvs[i]);
  }
  std::sort(reqs.begin(), reqs.end(), 
    [](auto const& a, auto const& b) {
      return a.kv.first < b.kv.first;
  });
  std::sort(existing_data.begin(), existing_data.end(), 
    [](auto const& a, auto const& b) {
      return a.first < b.first;
  });
  // Generate the requests based on the read-fraction
  for (int i = init_idx, j = init_idx, k = 0; i < tot_num; i += batch_size) {
    int batch_num = std::min(tot_num - i, batch_size);
    int num_read_per_batch = static_cast<int>(batch_num * read_frac);
    int num_write_per_batch = batch_num - num_read_per_batch;
    if (dist_name == "zipf") {
      ScrambledZipfianGenerator zipf_gen(existing_data.size());
      for (int u = 0; u < num_read_per_batch; ++ u) {
        int idx = zipf_gen.nextValue();
        reqs.push_back({kQuery, existing_data[idx]});
      }
    } else if (dist_name == "uniform") {
      std::mt19937_64 gen(kSEED);
      std::uniform_int_distribution<> uniform_gen(0, existing_data.size() - 1);
      for (int u = 0; u < num_read_per_batch; ++ u) {
        int idx = uniform_gen(gen);
        reqs.push_back({kQuery, existing_data[idx]});
      }      
    }
    int num_kks_write = static_cast<int>(num_write_per_batch * kks_frac);
    int num_oob_write = num_write_per_batch - num_kks_write;
    for (int u = 0; j < kks_idx && u < num_kks_write; ++ j, ++ u) {
      reqs.push_back({kInsert, kvs[j]});
      existing_data.push_back(kvs[j]);
    }
    for (int u = 0; k < oob_data.size() && u < num_oob_write; ++ k, ++ u) {
      reqs.push_back({kInsert, oob_data[k]});
      existing_data.push_back(oob_data[k]);
    }
  }
  write_requests(output_path, reqs);
}

int main(int argc, char* argv[]) {
    if (argc < 7) {
        std::cout << "No enough parameters for generating workloads based on the key set\n"
                  << "Please input: workload_gen (data_path) (distribution name) (batch_size) "
                  << "(ratio of data for bulk loading) (read ratio) (known-key-space write ratio)" << std::endl;
        exit(-1);
    }
    // know-key-space-write-ratio 用于选择 bulk loading 时 key 的[min, max]区间, 即 write 时 know-key-space 的 ration
    std::string data_path = std::string(argv[1]);
    std::string dist_name = std::string(argv[2]);
    int batch_size = std::stoi(argv[3]);
    double init_frac = ston<char*, double>(argv[4]);
    double read_frac = ston<char*, double>(argv[5]) / 100;
    double kks_frac = ston<char*, double>(argv[6]);

    std::string data_name = get_name(data_path);
    std::string workload_dir = data_path.substr(0, data_path.rfind("data/real")) + "workload";
    std::string output_path = path_join(workload_dir, data_name + "_" + str<int>(read_frac * 100) + "R_" + dist_name + ".bin");
    std::string source_path = data_path;
    generate_requests<double, long long>(output_path, source_path, dist_name, batch_size, init_frac, read_frac, kks_frac);
    return 0;
}