#!/bin/bash

en_type='partition'
de_type='sum'
#shifts=([longitudes-200M]=100000
#        [longlat-200M]=1000000
#        [ycsb-200M]=1000000
#        [lognormal-200M]=1000000
#        [books-200M]=1000000
#        [fb-200M]=10000000
#        [wiki-ts-200M]=1000000)
shift=1000000
num_flows=1
num_layers=2
input_dim=2
hidden_dim=1

data_dir=/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/data
data_name=longlat_200M_double_80R_zipf_10000-small
output_dir=/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/testdir
load_dir=/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/checkpoint/longlat_200M_double_80R_zipf_10000-small-2024-11-28-10-10-45/

python_file_path=/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/test.py

#conda activate jingtao8a_env
python3  ${python_file_path} --data_name=${data_name} --data_dir=${data_dir} --output_dir=${output_dir} --load_dir=${load_dir}\
            --encoder_type=${en_type} --decoder_type=${de_type} --shifts=${shift} \
            --num_flows=${num_flows} --num_layers=${num_layers} \
            --input_dim=${input_dim} --hidden_dim=${hidden_dim}

