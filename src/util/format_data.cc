#include "util/common.h"

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
    int end = data_path.rfind(".bin.data");
    int length = end - start;
    auto data_name = data_path.substr(start, length);
    return data_name;
}

std::string  get_data_type(std::string data_name) {
    return data_name.substr(data_name.rfind("_") + 1);
}

template<typename T>
T cut_key(T key, size_t len, bool verbose=false) {
  std::string key_str = str<T>(key);
  size_t start = key_str.size() < len ? 0 : key_str.size() - len;
  std::string key_cut_str = key_str.substr(start, len);
  if (verbose) {
    std::cout << std::fixed << std::setw(std::numeric_limits<T>::digits10) << key << std::endl;
    std::cout << std::fixed << std::setw(std::numeric_limits<T>::digits10) << key_str << std::endl;
    std::cout << std::fixed << std::setw(std::numeric_limits<T>::digits10) << key_cut_str << std::endl;
  }
  return ston<std::string, T>(key_cut_str);
}

template<typename T, typename P>
void format(std::string source_path, int num_keys = 0) {
  std::string output_path = source_path.substr(0, source_path.rfind('_')) + "_double.bin";

  std::ifstream in(source_path, std::ios::binary | std::ios::in);
  if (!in.is_open()) {
    std::cout << "File [" << source_path << "] does not exist" << std::endl;
    exit(-1);
  }
  std::cout << "First type bytes [" << str<size_t>(sizeof(T)) << "], " 
            << "second type bytes [" << str<size_t>(sizeof(P)) << "]" 
            << std::endl;
  std::vector<T> origin_keys;
  std::vector<P> double_keys;
  if (num_keys == 0) {
    in.read((char*)&num_keys, sizeof(T));
  }
  std::cout << "[" << num_keys << "] keys found in " << source_path << std::endl;
  origin_keys.resize(num_keys);
  in.read((char*)origin_keys.data(), num_keys * sizeof(T));
  in.close();

  if (sizeof(T) > sizeof(P) || (sizeof(T) == sizeof(P) && !std::numeric_limits<T>::is_signed)) {
    std::cout << "Cut keys" << std::endl;
    assert_p(std::numeric_limits<T>::is_integer, "Cannot cut the non-integer type");
    size_t key_len = std::numeric_limits<P>::digits10;
    for (int i = 0; i < origin_keys.size(); ++ i) {
      origin_keys[i] = cut_key<T>(origin_keys[i], key_len);
    }
  }
  std::sort(origin_keys.begin(), origin_keys.end());

  double_keys.reserve(num_keys);
  for (int i = 0; i < num_keys; ++ i) {
    if (i == 0 || !compare<T>(origin_keys[i], origin_keys[i - 1])) {
      double_keys.push_back(static_cast<P>(origin_keys[i]));
    }
  }
  int num_unique = double_keys.size();
  std::cout << "[" << num_unique << "] unique keys in " << source_path << std::endl;
  std::ofstream out(output_path, std::ios::binary | std::ios::out);
  out.write((char*)&num_unique, sizeof(int));
  out.write((char*)double_keys.data(), num_keys * sizeof(P));
  out.close();
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
      std::cout << "No enough parameters for formatting workloads\n"
                << "Please input: format (data_path)" << std::endl;
      exit(-1);
  }
  std::string data_path = std::string(argv[1]);
  std::string data_name = get_name(data_path);
  int num_keys = get_num_keys(data_name);
  std::string data_type = get_data_type(data_name);
  if (data_type == "uint64") {
    format<uint64_t, double>(data_path, num_keys);
  } else if (data_type == "double") {
    format<double, double>(data_path, num_keys);
  } else if (data_type == "int64") {
    format<long long, double>(data_path, num_keys);
  } else {
    std::cout << "Unspported data type [" << data_type << "]" << std::endl;
    exit(-1);
  }
  return 0;
}