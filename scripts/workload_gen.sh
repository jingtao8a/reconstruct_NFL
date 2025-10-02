#!/bin/bash
set -e # fail and exit on any command erroring

root_dir=$(cd `dirname $0`/..; pwd)
build_dir=${root_dir}/cmake-build-debug
exec=${build_dir}/workload_gen

data_path=/home/cg/chengang/jingtao8a/data/bin/ycsb_200M_double.bin
distribution_name=zipf
batch_size=10000
init_frac=0.5 # default
read_ratio=100
kks_write_frac=1 # default

echo "${exec} ${data_path} ${distribution_name} ${batch_size} ${init_frac} ${read_ratio} ${kks_write_frac}"
${exec} ${data_path} ${distribution_name} ${batch_size} ${init_frac} ${read_ratio} ${kks_write_frac}
