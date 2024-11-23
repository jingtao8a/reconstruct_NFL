#!/bin/bash
set -e # fail and exit on any command erroring

root_dir=$(cd `dirname $0`/..; pwd)
build_dir=${root_dir}/cmake-build-debug
exec=${build_dir}/nf_convert

workload_path=/home/chengang/chengang/jingtao8a/reconstruct_NFL/workload/longlat_200M_double_80R_zipf_10000.bin
proportion_of_keys=1
flow_input_director=/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/data

echo "${exec} ${workload_path} ${proportion_of_keys} ${flow_input_director}"
${exec} ${workload_path} ${proportion_of_keys} ${flow_input_director}
