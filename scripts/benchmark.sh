#!/bin/bash
set -e # fail and exit on any command erroring

root_dir=$(cd `dirname $0`/..; pwd)
build_dir=${root_dir}/cmake-build-debug
exec=${build_dir}/benchmark

index_name=nfl
workload_path=/home/cg/chengang/jingtao8a/reconstruct_NFL/workload/fb_200M_double_0R_zipf_10000.bin
config_path=/home/cg/chengang/jingtao8a/reconstruct_NFL/configs/fb_200M_double_0R_zipf_10000-weights.txt
show_incremental_updates=true

echo "${exec} ${index_name} ${workload_path} ${config_path} ${show_incremental_updates}"
${exec} ${index_name} ${workload_path} ${config_path} ${show_incremental_updates}