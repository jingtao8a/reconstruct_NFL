//
// Created by chengang on 24-11-27.
//

#include <torch/script.h>
#include <iostream>
#include <numeric>
#include "afli/conflicts.h"
#include "torch_models/numerical_flow.h"
using namespace std;

void readTxtData(std::string file_path, std::vector<double>& data_vector) {
    ifstream file(file_path);
    if (!file) {
        cerr << "无法打开文件: " << file_path << endl;
        return;
    }
    string line;
    double value;
    while (getline(file, line)) {
        stringstream line_stream(line);
        if (line_stream >> value) {
            data_vector.push_back(value);
        } else {
            cerr << "无法解析文件中的某一行数据" << endl;
        }
    }
    file.close();
}

void cut_conflictInfo(nfl::ConflictsInfo* p) {
    assert(p != NULL);
    for (int i = 0; i < p->num_conflicts_; ++i) {
        p->conflicts_[i] --;
    }
    int index = 0;
    for (; index < p->num_conflicts_; ++index) {
        if (p->conflicts_[index] > 0) {
            break;
        }
    }
    for (int i = index; i < p->num_conflicts_; ++i) {
        p->conflicts_[i - index] = p->conflicts_[i];
    }
    p->num_conflicts_ -= index;
}

void evaluate_keys(std::vector<double>& data) {
    std::cout << std::string(100, '*') << std::endl;
    std::cout << "Assess the" <<  data.size() << "of keys" << std::endl;
    std::sort(data.begin(), data.end());
    std::cout << "Range [" << std::setprecision(10) << data.front() << "," << data.back() << "]" << std::endl;
    std::vector<std::pair<double, uint64_t >> kvts;
    kvts.resize(data.size());
    for (int i = 0; i < data.size(); ++i) {
        kvts[i] = {data[i], 0};
    }
    double size_amp = 1.5;
    nfl::LinearModel<double>* model = new nfl::LinearModel<double>();
    auto pConflictsInfo = nfl::build_linear_model(kvts.data(), kvts.size(), model, size_amp);
    std::cout << "Slope " << model->slope_ << ", intercept " << model->intercept_ << std::endl;
    std::cout << "Space amplification " << size_amp << std::endl;
    std::sort(pConflictsInfo->conflicts_, pConflictsInfo->conflicts_ + pConflictsInfo->num_conflicts_);
    cut_conflictInfo(pConflictsInfo);
    auto totalConflicts = std::accumulate(pConflictsInfo->conflicts_, pConflictsInfo->conflicts_ + pConflictsInfo->num_conflicts_, 0);
    auto maxConflict =pConflictsInfo->conflicts_[std::max(0, int(pConflictsInfo->num_conflicts_ ) - 1)];
    auto tailConflict = pConflictsInfo->conflicts_[std::max(0, int(pConflictsInfo->num_conflicts_ * 0.99) - 1)];
    auto averageConflict = (double)totalConflicts / pConflictsInfo->num_conflicts_;
    std::cout << "Total conflicts " << totalConflicts << ", max conflict " << maxConflict <<
    ", 99% conflict " << tailConflict << ", average conflict " << averageConflict << std::endl;

    std::cout << std::endl;

    size_amp = 2;
    delete model;
    delete pConflictsInfo;
    model = new nfl::LinearModel<double>();
    pConflictsInfo = nfl::build_linear_model(kvts.data(), kvts.size(), model, size_amp);
    std::cout << "Slope " << model->slope_ << ", intercept " << model->intercept_ << std::endl;
    std::cout << "Space amplification " << size_amp << std::endl;
    std::sort(pConflictsInfo->conflicts_, pConflictsInfo->conflicts_ + pConflictsInfo->num_conflicts_);
    cut_conflictInfo(pConflictsInfo);
    totalConflicts = std::accumulate(pConflictsInfo->conflicts_, pConflictsInfo->conflicts_ + pConflictsInfo->num_conflicts_, 0);
    maxConflict =pConflictsInfo->conflicts_[std::max(0, int(pConflictsInfo->num_conflicts_ ) - 1)];
    tailConflict = pConflictsInfo->conflicts_[std::max(0, int(pConflictsInfo->num_conflicts_ * 0.99) - 1)];
    averageConflict = (double)totalConflicts / pConflictsInfo->num_conflicts_;
    std::cout << "Total conflicts " << totalConflicts << ", max conflict " << maxConflict <<
              ", 99% conflict " << tailConflict << ", average conflict " << averageConflict << std::endl;
    std::cout << std::string(100, '*') << std::endl;
}

void test1() {
    std::string path = "/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/checkpoint/longlat_200M_double_80R_zipf_10000-small-2024-11-28-10-10-45/";
    std::string script_model_path = path + "script_model.pt";
    torch::Device device(torch::kCUDA, 0);
    torch::jit::script::Module model = torch::jit::load(script_model_path);
    model.to(device);
    model.eval();

    std::string file_path = "/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/data/longlat_200M_double_80R_zipf_10000-small-training.txt";
    std::vector<double> data;
    readTxtData(file_path, data);
    evaluate_keys(data);
    double mean = -32419.0015482999988308;
    double var = 0.0101412717253000;

    int n = data.size();
    for (int i = 0; i < n; ++i) {
        data[i] = (data[i] - mean) / var;
    }
    auto x = torch::from_blob(data.data(), {n, 1}, torch::kDouble).to(device);
    std::vector<torch::jit::IValue> inputs;
    inputs.push_back(x);
    std::vector<double> tranData;
    tranData.resize(n);
    x = model.forward(inputs).toTensor();
    for (int i = 0; i < n; ++i) {
        tranData[i] = x[i][0].item<double>();
    }
    evaluate_keys(tranData);
}

void testNumericalFlow() {
    std::string model_dir("/home/chengang/chengang/jingtao8a/reconstruct_NFL/torch_model_weights/longlat_200M_double_zipf_10000_small");
    auto nflExp = nfl::NumericalFlow<double, uint64_t>(model_dir, 10000);
    std::pair<double, uint64_t> kv;
    kv.first = -20240.0;
    auto t_kv = nflExp.transform(kv);
    std::cout << t_kv.first << std::endl;
}
int main() {
    testNumericalFlow();
    return 0;
}
