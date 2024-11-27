import torch

torch.set_default_dtype(torch.float64)
torch.set_printoptions(precision=10, sci_mode=False)

def main():
    path = "/home/chengang/chengang/jingtao8a/reconstruct_NFL/train/checkpoint/longlat_200M_double_80R_zipf_10000-small-2024-11-27-11-15-46/checkpoint.pt"

    model_path = path.replace("checkpoint.pt", "model.pt")
    model = torch.load(model_path)
    mean_var_path = path.replace("checkpoint.pt", "mean_var.pt")
    mean_var = torch.load(mean_var_path)
    print(model.state_dict())
    print(mean_var.state_dict())

if __name__ == "__main__":
    main()