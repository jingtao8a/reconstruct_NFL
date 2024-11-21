#!/bin/bash
set -e # fail and exit on any command erroring

root_dir=$(cd `dirname $0`/..; pwd)
build_dir=${root_dir}/cmake-build-debug
exec=${build_dir}/data_gen
data_dir=/home/chengang/chengang/jingtao8a/reconstruct_NFL/data/synthetic
distribution_type=lognormal #uniform
num_keys=200
mean=0
var=2
begin=0
end=2147483648 # 2^31

echo "${exec} ${data_dir} ${distribution_type} ${num_keys} ${mean} ${var}"
${exec} ${data_dir} ${distribution_type} ${num_keys} ${mean} ${var}