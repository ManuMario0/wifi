import pandas as pd
import math
from datetime import datetime, timedelta
import matplotlib.pyplot as plt
import numpy as np

filter = ["e351313ce0f1d8855d96c7d3c7267c637ac208f02b60ba0ff8fee24502e66cb3",
"bc940849c23de5cdcbe9be9b064e479133a8a881d5e084e207d3d5bed9dbf91d",
"63037c3a59b5cba3498e93d31a662f3859d06e67e8d651c422539056fd3f3bd7",
"c0acda9b04e06b7dd1e78e4a56ce299dfbc7dc5d51a16e827f198322cc52ffd4",
"eea8691e6d1c98893e2050a4a7b8653df724dceb66bfca535ba2bfaa0e9547e1",
"0f0e41bc7c61f414ae324f321e1454efe99fc11c9eef0bf7fea3f8916f1df564",
"b0deb1287fb41428a52201328ed8e6d603b1ff94fc4b7fd7fa90af29104aa63c",
"d959d2090970f836a709a50f994c916a56218b23bbfad80d3374d5d914f18e7c",
"08ded64ef970defa187d3e45ece31dee935a54f282d7aa2bbd8532115e9aa17a",
"979c76b673df4d7585a874ddc06f33a80f7c477d328a112c1e9c3d854f2fec75"]


data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/fix_database.csv", parse_dates=["Radius-ts"])

data = data.set_index("Radius-ts", drop = False)

ap_diff = []
ap_swap = []

def check_diff(x, _ap_diff, _ap_swap, last_ap):
    if (x==0):
        return
    if x not in _ap_diff:
        _ap_diff.append(x)
        
    if x != last_ap[0]:
        _ap_swap[0] += 1
    
    last_ap[0] = x

def local(x):
    _ap_diff = []
    _ap_swap = [0]
    last_ap = [""]
    x.apply(check_diff, args=(_ap_diff, _ap_swap, last_ap,))
    
    return pd.DataFrame([[len(_ap_diff), _ap_swap[0]]])
    
    
def local2(x):
    if not x["User-ID"].iloc[0] in filter:
        return

    if (not isinstance(x["AP-MAC"].iloc[0], str)) or x["Elk-ts"].iloc[0] - x["Elk-ts"].iloc[-1] < 4*604800:
        return
    r = x.groupby(pd.Grouper(freq='1D'))["AP-MAC"].apply(local).mean()
    ap_diff.append(r.iloc[0])
    ap_swap.append(r.iloc[1])

data.groupby("User-MAC", sort=False, as_index=False).apply(local2)

im, ax = plt.subplots(figsize=(10, 10))
plt.xlim(-2, 62)
plt.ylim(-2, 62)

plt.scatter(ap_diff, ap_swap)
plt.plot(np.arange(0, 70, 1), 'r')
plt.title("AP swap/AP count")
plt.xlabel("AP count")
plt.ylabel("AP swap")
plt.savefig("scatter.png")
plt.clf()
plt.close()

ap_diff.sort()
plt.plot(ap_diff, '.')
plt.title("AP count")
plt.savefig("diff.png")
plt.clf()

ap_swap.sort()
plt.plot(ap_swap, '.')
plt.title("AP swap")
plt.savefig("swap.png")
plt.clf()
