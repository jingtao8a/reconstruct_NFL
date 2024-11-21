#!/bin/bash
set -e # fail and exit on any command erroring

root_dir=$(cd `dirname $0`/..; pwd)
build_dir=${root_dir}/cmake-build-debug
exec=${build_dir}/format
data_path=/home/chengang/chengang/jingtao8a/reconstruct_NFL/data/real/int64_no_prefix/ycsb_200M_int64.bin.data

echo "${exec} ${data_path}"
${exec} ${data_path}

