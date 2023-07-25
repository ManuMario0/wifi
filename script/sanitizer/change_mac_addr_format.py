import pandas as pd
import math

data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/devices.csv")

def local(x):
    return x.replace(':', '').upper()

data["BaseRadio MAC Address"] = data["BaseRadio MAC Address"].apply(local)

data.to_csv("/Volumes/Emmanuel/stage[DELETE]/fix_devices.csv", index=False)
