import torch

from distribution_transformer import DistTransformer
from mean_var import MeanVar

torch.set_default_dtype(torch.float64)
torch.set_printoptions(precision=10, sci_mode=False)

def test1(path):
    encoder_config = {}
    decoder_config = {}
    encoder_config['shifts'] = 1000000
    encoder_config['keep_res'] = True
    model = DistTransformer(1, 2, 2,
                            1, "partition", encoder_config,
                            "sum", decoder_config)
    model_path = path.replace("checkpoint.pt", "model.pt")
    model.load_state_dict(torch.load(model_path))
    mean_var_path = path.replace("checkpoint.pt", "mean_var.pt")
    mean_var = MeanVar()
    mean_var.load_state_dict(torch.load(mean_var_path))
    print(model.state_dict())
    print(mean_var.state_dict())

def test2(path):
    model_path = path.replace("checkpoint.pt", "entire_model.pt")
    model = torch.load(model_path)
    print(model.state_dict())

def test3(path):
    script_model_path = path.replace("checkpoint.pt", "script_model.pt")
    model = torch.jit.load(script_model_path)
    print(model.state_dict())

def main():
    path = "/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/checkpoint/longlat_200M_double_80R_zipf_10000-2024-11-28-11-52-10/checkpoint.pt"
    test1(path)
    test2(path)
    test3(path)

if __name__ == "__main__":
    main()