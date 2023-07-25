import pandas as pd
import math

data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/wifi_2022-23.csv")

def local(x):
    return math.floor(pd.Timestamp(x).timestamp())

data["Elk-ts"] = data["Radius-ts"].apply(local)

data.to_csv("/Volumes/Emmanuel/stage[DELETE]/fix_database.csv", index=False)
