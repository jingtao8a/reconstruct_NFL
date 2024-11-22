#!/bin/bash
set -e # fail and exit on any command erroring

root_dir=$(cd `dirname $0`/..; pwd)
build_dir=${root_dir}/cmake-build-debug
exec=${build_dir}/format
data_path=/home/chengang/chengang/jingtao8a/reconstruct_NFL/data/real/uint64_no_prefix/fb_200M_uint64.bin.data

echo "${exec} ${data_path}"
${exec} ${data_path}

