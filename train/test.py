import os
import argparse
import torch
import time
import sys
import random
from tqdm import trange
from data_util import *
from distribution_transformer import *
from assess_quality import *
from mean_var import MeanVar

torch.set_default_dtype(torch.float64)
torch.set_printoptions(precision=10, sci_mode=False)

def load(model, mean_var, path):
    checkpoint = torch.load(path)
    model.load_state_dict(checkpoint["model"])
    mean_var.load_state_dict(checkpoint["mean_var"])

if __name__ == '__main__':
    # Initializing parameters for flow models
    parser = argparse.ArgumentParser()
    parser.add_argument('--device', type=str, default='cuda:0')
    parser.add_argument('--data_dir', type=str, default='')
    parser.add_argument('--data_name', type=str, default='')
    parser.add_argument('--plot', type=bool, default=True)
    parser.add_argument('--log_file', type=bool, default=True)
    parser.add_argument('--output_dir', type=str, default='')
    parser.add_argument('--load_dir', type=str, default='')

    # Parameters for encoder/decoder
    parser.add_argument('--encoder_type', type=str, default='partition')
    parser.add_argument('--decoder_type', type=str, default='sum')
    parser.add_argument('--shifts', type=int, default=1000000)
    parser.add_argument('--keep_res', type=bool, default=True)
    parser.add_argument('--reduce_dim', type=int, default=-1)

    # Parameters for flow models
    parser.add_argument("--num_flows", type=int, default=1)
    parser.add_argument('--num_layers', type=int, default=2)
    parser.add_argument('--input_dim', type=int, default=1)
    parser.add_argument('--hidden_dim', type=int, default=1)

    args = parser.parse_args()
    if len(args.output_dir) == 0:
        print("you did not input output_dir")
        sys.exit(-1)

    if not os.path.exists(args.output_dir):
        os.mkdir(args.output_dir)

    if args.log_file:
        log_filename = os.path.join(args.output_dir, 'outputs.log')
        log_f = open(log_filename, 'w')
    else:
        log_f = None

    # Loading data, analyzing data, segmenting data
    print('Process: Loading data...')
    start_time = time.time()
    load_keys = load_data(os.path.join(args.data_dir, args.data_name), 'training')
    load_durations = time.time() - start_time
    print('Time Cost of Loading data {}'.format(load_durations))

    print('Process: Evaluating original keys [{}]...'.format(load_keys.size()))
    evaluate_keys(load_keys.detach().cpu().numpy().squeeze(), log_f=log_f)

    encoder_config = {}
    decoder_config = {}
    if args.encoder_type == 'partition':
        encoder_config['shifts'] = args.shifts
        encoder_config['keep_res'] = args.keep_res

    if args.decoder_type == 'reduce':
        decoder_config['reduce_dim'] = args.reduce_dim
    elif args.decoder_type == 'shift_sum':
        decoder_config['shifts'] = args.shifts
        decoder_config['keep_res'] = args.keep_res
    print('Encoder configs\t{}'.format(encoder_config))
    print('Decoder configs\t{}'.format(decoder_config))
    if args.log_file:
        print('Encoder configs\t{}'.format(encoder_config), file=log_f)
        print('Decoder configs\t{}'.format(decoder_config), file=log_f)

    model = DistTransformer(args.num_flows, args.num_layers, args.input_dim,
                            args.hidden_dim, args.encoder_type, encoder_config,
                            args.decoder_type, decoder_config)
    mean_var = MeanVar()
    print('Parameters={}, n_dims={}'.format(sum((p != 0).sum()
                                                if len(p.shape) > 1 else torch.tensor(p.shape).item()
                                                for p in model.parameters()), args.input_dim))
    print('{}'.format(model))
    if args.log_file:
        print('Parameters={}, n_dims={}'.format(sum((p != 0).sum()
                                                    if len(p.shape) > 1 else torch.tensor(p.shape).item()
                                                    for p in model.parameters()), args.input_dim), file=log_f)
        oldstdout = sys.stdout
        sys.stdout = log_f
        print('{}'.format(model))
        sys.stdout = oldstdout

    if args.load_dir == '':
        print("you did not input load_dir")
        sys.exit(-1)
    load_path = os.path.join(args.load_dir, 'checkpoint.pt')
    load(model, mean_var, load_path)

    # Min-Max归一化操作目的是将 keys 全部转换为正数
    print('Process: Using Min-Max Normalization')
    global_mean = mean_var.mean
    global_var = mean_var.var
    load_keys = (load_keys - global_mean) / global_var
    print('Min {} Max {}'.format(torch.min(load_keys), torch.max(load_keys)))
    if args.log_file:
        print('Min {} Max {}'.format(torch.min(load_keys), torch.max(load_keys)), file=log_f)

    print('Process: Transforming keys...')
    model = model.to(args.device)
    model.eval()
    tran_keys = None
    with torch.no_grad():
        batch_size = 8192
        num_batches = int(np.ceil(load_keys.shape[0] / batch_size))
        iterator = trange(num_batches, smoothing=0, dynamic_ncols=True)
        for i in iterator:
            l = i * batch_size
            r = np.min([(i + 1) * batch_size, load_keys.shape[0]])
            x = load_keys[l:r].to(args.device)
            z = model(x).to('cpu')
            if torch.isnan(z).any():
                print('Error: Nan keys')
                exit()
            tran_keys = torch.cat((tran_keys, z), 0) if tran_keys != None else z

    num_unordered = ((tran_keys[1:] - tran_keys[:-1]) < 0).sum().item()
    if num_unordered > 0:
        print('Process: Sorting keys...')
        tran_keys = torch.sort(tran_keys, 0)[0]

    print('Process: Evaluating the transformed keys...')
    evaluate_keys(tran_keys.detach().cpu().numpy().squeeze(), log_f=log_f)