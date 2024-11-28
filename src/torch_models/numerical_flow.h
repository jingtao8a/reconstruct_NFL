//
// Created by chengang on 24-11-26.
//

#ifndef RECONSTRUCT_NFL_NUMERICAL_FLOW_H
#define RECONSTRUCT_NFL_NUMERICAL_FLOW_H

#include <torch/script.h>
#include "util/common.h"

namespace nfl {

    template<typename KT, typename VT>
    class NumericalFlow {
        typedef std::pair<KT, VT> KVT;
        typedef std::pair<KT, KVT> KKVT;
    public:
        double mean_;
        double var_;
        int batch_size_;
        torch::Device device;
        torch::jit::script::Module model;

    public:
        explicit NumericalFlow(std::string model_dir, uint32_t batch_size): batch_size_(batch_size), device(torch::kCUDA, 0) {
            std::string model_path = path_join(model_dir, "script_model.pt");
            std::string mean_var_path = path_join(model_dir, "mean_var.txt");
            model = torch::jit::load(model_path);
            model.to(device);
            model.eval();
            //mean_ var_的初始化
            std::vector<KT> data_vector;
            std::ifstream file(mean_var_path);
            if (!file) {
                std::cerr << "无法打开文件: " << mean_var_path << std::endl;
                return;
            }
            std::string line;
            KT value;
            while (getline(file, line)) {
                std::stringstream line_stream(line);
                if (line_stream >> value) {
                    data_vector.push_back(value);
                } else {
                    std::cerr << "无法解析文件中的某一行数据" << std::endl;
                }
            }
            file.close();
            mean_ = data_vector[0];
            var_ = data_vector[1];
        }

        uint64_t size() {
            return sizeof(NumericalFlow<KT, VT>) + sizeof(model);
        }

        void set_batch_size(uint32_t batch_size) {
            batch_size_ = batch_size;
        }

        void transform(const KVT* kvs, uint32_t size, KKVT* tran_kvs) {
            std::vector<KT> data;
            data.resize(size);
            for (uint32_t i = 0; i < size; ++ i) {
                tran_kvs[i] = {(kvs[i].first - mean_) / var_, kvs[i]};
                data[i] = tran_kvs[i].first;
            }
            uint32_t num_batches = static_cast<uint32_t>(std::ceil(size * 1. / batch_size_));
            for (uint32_t i = 0; i < num_batches; ++ i) {
                uint32_t l = i * batch_size_;
                uint32_t r = std::min((i + 1) * batch_size_, size);
                torch::Tensor x = torch::from_blob(data.data() + l, {r - l, 1}, torch::kDouble).to(device);
                std::vector<torch::jit::IValue> inputs;
                inputs.push_back(x);
                x = model.forward(inputs).toTensor().to("cpu");
                for (auto j = l; j < r; ++j) {
                    tran_kvs[j].first = x[j - l][0].item<double>();
                }
            }
        }

        KKVT transform(const KVT kv) {
            KKVT t_kv = {(kv.first - mean_) / var_, kv};
            std::vector<KT> data;
            data.push_back(t_kv.first);
            torch::Tensor x = torch::from_blob(data.data(), {1, 1}, torch::kDouble).to(device);
            std::vector<torch::jit::IValue> inputs;
            inputs.push_back(x);
            x = model.forward(inputs).toTensor().to("cpu");
            t_kv.first = x[0][0].item<double>();
            return t_kv;
        }
    };
}


#endif //RECONSTRUCT_NFL_NUMERICAL_FLOW_H
