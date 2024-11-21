#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "util/workload.h"
#include "util/common.h"

#include "afli/afli.h"
#include "nfl/nfl.h"
#include "lipp/src/core/lipp.h"
#include "ALEX/src/core/alex.h"

namespace nfl {
//configs 目录下的文件中可以添加 bucket_size aggregate_size (key)
struct AFLIConfig {
  int bucket_size;
  int aggregate_size;

  AFLIConfig(std::string path) {
    bucket_size = -1;
    aggregate_size = 0;
    if (path != "") {
      std::ifstream in(path, std::ios::in);
      if (in.is_open()) {
        while (!in.eof()) {
          std::string kv;
          in >> kv;
          std::string::size_type n = kv.find("=");
          if (n != std::string::npos) {
            std::string key = kv.substr(0, n);
            std::string val = kv.substr(n + 1);
            if (key == "bucket_size") {
              bucket_size = std::stoi(val);
            } else if (key == "aggregate_size") {
              aggregate_size = std::stoi(val);
            }
          }
        }
        in.close();
      }
    }
  }
};

struct NFLConfig {
  int bucket_size;
  int aggregate_size;
  std::string weights_path;

  NFLConfig(std::string path) {
    bucket_size = -1;
    aggregate_size = 0;
    weights_path = "";
    if (path != "") {
      std::ifstream in(path, std::ios::in);
      if (in.is_open()) {
        while (!in.eof()) {
          std::string kv;
          in >> kv;
          std::string::size_type n = kv.find("=");
          if (n != std::string::npos) {
            std::string key = kv.substr(0, n);
            std::string val = kv.substr(n + 1);
            if (key == "bucket_size") {
              bucket_size = std::stoi(val);
            } else if (key == "aggregate_size") {
              aggregate_size = std::stoi(val);
            } else if (key == "weights_path") {
              weights_path = val;
            }
          }
        }
        in.close();
      }
    }
  }
};

template<typename KT, typename VT>
class Benchmark {
typedef std::pair<KT, VT> KVT;

public:
  
  std::vector<KVT> init_data;//bulk load data的数据
  std::vector<Request<KT, VT>> requests;//Read Write Operation
  const double conflicts_decay = 0.1;

  void run_workload(std::string index_name, int batch_size, 
                    std::string workload_path, std::string config_path="",
                    bool show_incremental_throughputs=false) {
    init_data.clear();
    requests.clear();
    std::string workload_name = get_workload_name(workload_path);
    load_data(workload_path, init_data, requests);
    // Check the order of load data.
    for (int i = 1; i < init_data.size(); ++ i) {
      if (init_data[i].first < init_data[i - 1].first || 
          compare(init_data[i].first, init_data[i - 1].first)) {
        std::cout << std::fixed 
                  << std::setprecision(std::numeric_limits<KT>::digits10) 
                  << "Duplicated data" << std::endl << i - 1 << "th key [" 
                  << init_data[i - 1].first << "]" << std::endl << i 
                  << "th key [" << init_data[i].first << "]" << std::endl;
      }
      assert_p(init_data[i].first > init_data[i - 1].first, "Unordered case in load data");
    }
    // Start to evaluate
    bool show_stat = true;
    ExperimentalResults exp_res(batch_size);
    if (start_with(index_name, "afli")) {
        run_afli(batch_size, exp_res, config_path, show_stat);
    } else if (start_with(index_name, "nfl")) {
        run_nfl(batch_size, exp_res, config_path, show_stat);
    } else if (start_with(index_name, "lipp")) {
        run_lipp(batch_size, exp_res, config_path, show_stat);
    } else if (start_with(index_name, "alex")) {
        run_alex(batch_size, exp_res, config_path, show_stat);
    } else {
        std::cout << "Unsupported model name [" << index_name << "]" << std::endl;
        exit(-1);
    }
    // Print results.
    std::cout << workload_name << "\t" << index_name << "\t" << batch_size 
              << std::endl;
    if (show_incremental_throughputs) {
      exp_res.show_incremental_throughputs();
    } else {
      exp_res.show();
    }
  }

  void run_afli(int batch_size, ExperimentalResults& exp_res, 
                std::string config_path, bool show_stat=false) {
    AFLIConfig config(config_path);
    // Start to bulk load
    auto bulk_load_start = std::chrono::high_resolution_clock::now();
    AFLI<KT, VT> afli;
    afli.bulk_load(init_data.data(), init_data.size());
    auto bulk_load_end = std::chrono::high_resolution_clock::now();
    exp_res.bulk_load_index_time = 
      std::chrono::duration_cast<std::chrono::nanoseconds>(bulk_load_end 
                                                    - bulk_load_start).count();
    if (show_stat) {
      afli.print_stats();
    }

    std::vector<KVT> batch_data;
    batch_data.reserve(batch_size);
    // Perform requests in batch
    int num_batches = std::ceil(requests.size() * 1. / batch_size);
    exp_res.latencies.reserve(num_batches * 3);
    exp_res.need_compute.reserve(num_batches * 3);
    for (int batch_idx = 0; batch_idx < num_batches; ++ batch_idx) {
      batch_data.clear();
      int l = batch_idx * batch_size;
      int r = std::min((batch_idx + 1) * batch_size, 
                        static_cast<int>(requests.size()));
      for (int i = l; i < r; ++ i) {
        batch_data.push_back(requests[i].kv);
      }

      VT val_sum = 0;
      // Perform requests
      auto start = std::chrono::high_resolution_clock::now();
      for (int i = l; i < r; ++ i) {
        int data_idx = i - l;
        if (requests[i].op == kQuery) {
          auto it = afli.find(batch_data[data_idx].first);
          if (!it.is_end()) {
            val_sum += it.value();
          }
        } else if (requests[i].op == kUpdate) {
          bool res = afli.update(batch_data[data_idx]);
        } else if (requests[i].op == kInsert) {
          afli.insert(batch_data[data_idx]);
        } else if (requests[i].op == kDelete) {
          int res = afli.remove(batch_data[data_idx].first);
        }
      }
      auto end = std::chrono::high_resolution_clock::now();
      double time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
      exp_res.sum_indexing_time += time;
      exp_res.num_requests += batch_data.size();
      exp_res.latencies.push_back({0, time});
      exp_res.step();
    }
    exp_res.model_size = afli.model_size();
    exp_res.index_size = afli.index_size();
    if (show_stat) {
      afli.print_stats();
    }
  }

  void run_nfl(int batch_size, ExperimentalResults& exp_res, 
                std::string config_path, bool show_stat=false) {
    NFLConfig config(config_path);
    // Start to bulk load
    auto bulk_load_start = std::chrono::high_resolution_clock::now();
    NFL<KT, VT> nfl(config.weights_path, batch_size);
    uint32_t tail_conflicts = nfl.auto_switch(init_data.data(), 
                                              init_data.size());
    auto bulk_load_mid = std::chrono::high_resolution_clock::now();
    nfl.bulk_load(init_data.data(), init_data.size(), tail_conflicts);
    auto bulk_load_end = std::chrono::high_resolution_clock::now();
    exp_res.bulk_load_trans_time = // init_data 转换时间
      std::chrono::duration_cast<std::chrono::nanoseconds>(bulk_load_mid 
                                                    - bulk_load_start).count();
    exp_res.bulk_load_index_time = // init_data 加载时间
      std::chrono::duration_cast<std::chrono::nanoseconds>(bulk_load_end 
                                                      - bulk_load_mid).count();
    if (show_stat) {
      nfl.print_stats();
    }

    std::vector<KVT> batch_data;
    batch_data.reserve(batch_size);
    // Perform requests in batch
    int num_batches = std::ceil(requests.size() * 1. / batch_size);
    exp_res.latencies.reserve(num_batches * 3);
    exp_res.need_compute.reserve(num_batches * 3);
    for (int batch_idx = 0; batch_idx < num_batches; ++ batch_idx) {
      batch_data.clear();
      int l = batch_idx * batch_size;
      int r = std::min((batch_idx + 1) * batch_size, 
                        static_cast<int>(requests.size()));
      for (int i = l; i < r; ++ i) {
        batch_data.push_back(requests[i].kv);
      }

      VT val_sum = 0;
      // Perform requests
      auto start = std::chrono::high_resolution_clock::now();
      nfl.transform(batch_data.data(), batch_data.size());
      auto mid = std::chrono::high_resolution_clock::now();
      for (int i = l; i < r; ++ i) {
        int data_idx = i - l;
        if (requests[i].op == kQuery) {
          auto it = nfl.find(data_idx);
          if (!it.is_end()) {
              val_sum += it.value();
          }
        } else if (requests[i].op == kUpdate) {
          bool res = nfl.update(data_idx);
        } else if (requests[i].op == kInsert) {
          nfl.insert(data_idx);
        } else if (requests[i].op == kDelete) {
          int res = nfl.remove(data_idx);
        }
      }
      auto end = std::chrono::high_resolution_clock::now();
      double time1 = std::chrono::duration_cast<std::chrono::nanoseconds>(mid 
                                                              - start).count();
      double time2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end 
                                                              - mid).count();
      exp_res.sum_transform_time += time1;//read write 转换时间
      exp_res.sum_indexing_time += time2;//read write 查找和插入时间
      exp_res.num_requests += batch_data.size();
      exp_res.latencies.push_back({time1, time2});
      exp_res.step();
    }
    exp_res.model_size = nfl.model_size();
    exp_res.index_size = nfl.index_size();
    if (show_stat) {
      nfl.print_stats();
    }
  }

  void run_lipp(int batch_size, ExperimentalResults& exp_res,
                 std::string config_path, bool show_stat=false) {
    // Start to bulk load
      auto bulk_load_start = std::chrono::high_resolution_clock::now();
      LIPP<KT, VT> lipp;
      lipp.bulk_load(init_data.data(), init_data.size());
      auto bulk_load_end = std::chrono::high_resolution_clock::now();
      exp_res.bulk_load_index_time =
              std::chrono::duration_cast<std::chrono::nanoseconds>(bulk_load_end
                                                                   - bulk_load_start).count();
      std::vector<KVT> batch_data;
      batch_data.reserve(batch_size);
      // Perform requests in batch
      int num_batches = std::ceil(requests.size() * 1. / batch_size);
      exp_res.latencies.reserve(num_batches * 3);
      exp_res.need_compute.reserve(num_batches * 3);

      for (int batch_idx = 0; batch_idx < num_batches; ++ batch_idx) {
          batch_data.clear();
          int l = batch_idx * batch_size;
          int r = std::min((batch_idx + 1) * batch_size,
                           static_cast<int>(requests.size()));
          for (int i = l; i < r; ++ i) {
              batch_data.push_back(requests[i].kv);
          }

          VT val_sum = 0;
          // Perform requests
          auto start = std::chrono::high_resolution_clock::now();
          for (int i = l; i < r; ++ i) {
              int data_idx = i - l;
              if (requests[i].op == kQuery) {
                  auto it = lipp.at(batch_data[data_idx].first);
                  val_sum += it;
              } else if (requests[i].op == kInsert) {
                  lipp.insert(batch_data[data_idx]);
              }
          }
          auto end = std::chrono::high_resolution_clock::now();
          double time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
          exp_res.sum_indexing_time += time;
          exp_res.num_requests += batch_data.size();
          exp_res.latencies.push_back({0, time});
          exp_res.step();
      }
      exp_res.model_size = lipp.model_size();
      exp_res.index_size = lipp.index_size();
      if (show_stat) {
          lipp.print_depth();
      }
  }

    void run_alex(int batch_size, ExperimentalResults& exp_res,
                  std::string config_path, bool show_stat=false) {
        // Start to bulk load
        auto bulk_load_start = std::chrono::high_resolution_clock::now();
        alex::Alex<KT, VT> alex;
        alex.bulk_load(init_data.data(), init_data.size());
        auto bulk_load_end = std::chrono::high_resolution_clock::now();
        exp_res.bulk_load_index_time =
                std::chrono::duration_cast<std::chrono::nanoseconds>(bulk_load_end
                                                                     - bulk_load_start).count();

        std::vector<KVT> batch_data;
        batch_data.reserve(batch_size);
        // Perform requests in batch
        int num_batches = std::ceil(requests.size() * 1. / batch_size);
        exp_res.latencies.reserve(num_batches * 3);
        exp_res.need_compute.reserve(num_batches * 3);
        for (int batch_idx = 0; batch_idx < num_batches; ++ batch_idx) {
            batch_data.clear();
            int l = batch_idx * batch_size;
            int r = std::min((batch_idx + 1) * batch_size,
                             static_cast<int>(requests.size()));
            for (int i = l; i < r; ++ i) {
                batch_data.push_back(requests[i].kv);
            }

            VT val_sum = 0;
            // Perform requests
            auto start = std::chrono::high_resolution_clock::now();
            for (int i = l; i < r; ++ i) {
                int data_idx = i - l;
                if (requests[i].op == kQuery) {
                    auto res = alex.find(batch_data[data_idx].first);
                    if (res != alex.end()) {
                        val_sum += res.payload();
                    }
                } else if (requests[i].op == kUpdate) {
                    auto res = alex.find(batch_data[data_idx].first);
                    if (res != alex.end()) {
                        res.payload() = batch_data[data_idx].second;
                    }
                } else if (requests[i].op == kInsert) {
                    alex.insert(batch_data[data_idx].first, batch_data[data_idx].second);
                } else if (requests[i].op == kDelete) {
                    int res = alex.erase(batch_data[data_idx].first);
                }
            }
            auto end = std::chrono::high_resolution_clock::now();
            double time = std::chrono::duration_cast<std::chrono::nanoseconds>(end
                                                                               - start).count();
            exp_res.sum_indexing_time += time;
            exp_res.num_requests += batch_data.size();
            exp_res.latencies.push_back({0, time});
            exp_res.step();
        }
        exp_res.model_size = alex.model_size();
        exp_res.index_size = alex.model_size() + alex.data_size();
        if (show_stat) {
            alex.print_stats();
        }
    }
};

}

#endif