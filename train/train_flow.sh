#!/bin/bash

# Parameters for train the numerical flow
#seeds=([longitudes-200M]=1000000013
#        [longlat-200M]=1000000007
#        [ycsb-200M]=1000000013
#        [lognormal-200M]=1000000007
#        [books-200M]=1000000007
#        [fb-200M]=1000000003
#        [wiki-ts-200M]=1000000013)
seed=1000000007
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
train_ratio=0.1
num_train=3
learning_rate=0.1
steps=15
batch_dim=4096
loss_func='normal'
output_dir=/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/checkpoint

data_name=longlat_200M_double_80R_zipf_10000-small
data_dir=/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/data

python_file_path=/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/numerical_flow.py
#conda activate jingtao8a_env
python3  ${python_file_path} --data_name=${data_name} --data_dir=${data_dir}\
            --seed=${seed} --encoder_type=${en_type} \
            --decoder_type=${de_type} --shifts=${shift} \
            --num_flows=${num_flows} --num_layers=${num_layers} \
            --input_dim=${input_dim} --hidden_dim=${hidden_dim} \
            --train_ratio=${train_ratio} --num_train=${num_train} \
            --learning_rate=${learning_rate} --steps=${steps} \
            --batch_dim=${batch_dim} --loss_func=${loss_func} \
            --output_dir=${output_dir}
