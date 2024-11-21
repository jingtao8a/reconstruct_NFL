# 可执行程序的参数选择
## format
- 将 data/real 下的真实数据集 format 成 double类型, 可能需要进行截断和去重操作 
- format 之前的数据集后缀为 .bin.data
- format 之后的数据集后缀为 .bin
- format 前后的数据在同一目录下
```
format (data_path)
```
## data_gen
- 生成合成数据集
- 生成数据集的目录为 data/synthetic
- 生成数据集的数据类型为 double
```
# Configurations for synthetic dataset
mean=0
var_list=(1 2 4 8)
begin=0
end=2147483648 # 2^31

data_gen (data direcotry) lognormal (number millions of keys) (mean) (variance)
data_gen (data directory) uniform (number millions of keys) (begin) (end)
```

## workload_gen
- 生成工作负载
- 可以指定 bulk loading ratio、read/write ratio、know-key-space write ratio
- 可以选择 read 操作时使用的分布（zip or uniform）, 以及批量操作 batch_size
```
# Configurations for synthetic workloads
req_dist_list=('zipf', 'uniform') //distribution name
batch_size=256 //batch_size
init_frac_list=(0.5) //ratio of data for bulk loading 
read_frac_list=(100 80 20 0) //read ratio
kks_frac_list=(1) //known-key-space write ratio

workload_gen (data_path) (distribution name) (batch_size) (ratio of data for bulk loading) (read ratio) (known-key-space write ratio)"
```

## benchmark
- 测试基准
```
# Configurations for benchmark
algorithms=('nfl' 'afli') // index name
batch_size=256 // batch size
workload_path=/home/chengang/chengang/jingtao8a/reconstruct_NFL/workload/ycsb_200M_double_80R_zipf.bin
config_path=/home/chengang/chengang/jingtao8a/reconstruct_NFL/configs/nfl_ycsb_200M_double.in // config_path
show_incremental_updates=any // show incremental updates

benchmark (index name) (batch size) (workload path) [config path] [show incremental updates]
```
