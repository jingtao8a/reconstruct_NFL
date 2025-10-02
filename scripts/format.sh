#!/bin/bash
set -e # fail and exit on any command erroring

root_dir=$(cd `dirname $0`/..; pwd)
build_dir=${root_dir}/cmake-build-debug
exec=${build_dir}/format
data_path=/home/cg/chengang/jingtao8a/data/bin.data/int64/ycsb_200M_int64.bin.data

echo "${exec} ${data_path}"
${exec} ${data_path}

