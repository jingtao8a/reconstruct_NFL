# 可执行程序的参数选择

## gen
### 生成合成数据集
```
# Configurations for synthetic dataset
mean=0
var_list=(1 2 4 8)
begin=0
end=2147483648 # 2^31

gen dataset (data directory) lognormal (number millions of keys) (mean) (variance)

gen dataset (data directory) uniform (number millions of keys) (begin) (end)
```

### 将真实数据集format成double类型
```
# Configurations for synthetic workloads
init_frac_list=(0.5)
read_frac_list=(100 80 20 0)
kks_frac_list=(1)
# Configurations for realistic workloads
float64_workloads=('longlat-200M' 'longitudes-200M')
int64_workloads=('ycsb-200M')
uint64_workloads=('fb-200M' 'lognormal-200M' 'books-200M' 'wiki-ts-200M')


```

## format


## benchmark