#!/bin/bash
set -e # fail and exit on any command erroring

root_dir=$(cd `dirname $0`/..; pwd)
build_dir=${root_dir}/cmake-build-debug
exec=${build_dir}/benchmark

index_name=nfl
batch_size=256
workload_path=/home/chengang/chengang/jingtao8a/reconstruct_NFL/workload/ycsb_200M_double_80R_zipf.bin
config_path=/home/chengang/chengang/jingtao8a/reconstruct_NFL/configs/nfl.in
show_incremental_updates=""

echo "${exec} ${index_name} ${batch_size} ${workload_path} ${config_path} ${show_incremental_updates}"
${exec} ${index_name} ${batch_size} ${workload_path} ${config_path} ${show_incremental_updates}