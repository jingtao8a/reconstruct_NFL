class MeanVar:
    def __init__(self):
        self.mean = None
        self.var = None

    def state_dict(self):
        return {"mean" : self.mean, "var": self.var }

    def load_state_dict(self, dict):
        self.mean = dict["mean"]
        self.var = dict["var"]