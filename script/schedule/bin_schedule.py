import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime, timedelta
import numpy as np
import os
import math

data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/fix_database_discretize.csv", parse_dates=["Radius-ts"])
data = data.set_index("Radius-ts", drop = False)

devices_translation_data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/fix_devices.csv")

devices_translation = {}

def assign(x):
    devices_translation[x[0]] = x[1]

devices_translation_data[["BaseRadio MAC Address", "AP Name"]].apply(assign, axis = 1)

del devices_translation_data

significant_data = {}

def local(x, hist):
    hist[x["Radius-ts"].hour * 4 + x["Radius-ts"].minute // 15] += 1

hmax = 0
def extract_significant_data(x):
    hist = np.zeros(24*4)# [0 for i in range(24*4)]
    x.apply(local, axis=1, args=(hist,))
    
    global hmax
    hmax = max(hmax, max(hist))
    if (max(hist) > 0):
        significant_data[x["AP-MAC"].iloc[0]] = hist#/max(hist)

data.groupby(["AP-MAC"]).apply(extract_significant_data)
for key, val in significant_data.items():
    significant_data[key] = val/hmax

id1 = "c0acda9b04e06b7dd1e78e4a56ce299dfbc7dc5d51a16e827f198322cc52ffd4"
#id2 = "b0deb1287fb41428a52201328ed8e6d603b1ff94fc4b7fd7fa90af29104aa63c"
#id2 = "eea8691e6d1c98893e2050a4a7b8653df724dceb66bfca535ba2bfaa0e9547e1"
id2 = "c3eee99abf8649fff24ec3578ba454b29dbf5b253fa1a7c0aa7799a560ca26b1"

mac1 = "c472f306ab2f978268b84bdf2656d018e0bb1fa5f315dbd9e79a6836f8162aaa"
#mac2 = "f2e4ff874ba141a118fc7b08f3e04957def9e9a9c603bd1c9013117128073278"
#mac2 = "1ccf8b8e1e2f738ca397d1f60ed95bbaadcef796ffee21ceb91c2cb79f19864b"
mac2 = "301de9a4c87077c0715d1675b30989781a6f25e74d90cb42414c1d755da855c5"

def extract_device(x):
    id = x["User-ID"].iloc[0]
    mac = x["User-MAC"].iloc[0]
    global data1
    global data2
    if (id == id1 and mac == mac1):
        data1 = x["AP-MAC"]
    if (id == id2 and mac == mac2):
        data2 = x["AP-MAC"]

data.groupby(["User-ID", "User-MAC"]).apply(extract_device)

df = pd.merge(data1, data2, left_index=True, right_index=True)

def seven_differencies(x):
    if (x["AP-MAC_x"] == x["AP-MAC_y"]):
#        print(str(x.name) + " : " + devices_translation[x["AP-MAC_x"]] + " ; " + str(significant_data[x["AP-MAC_x"]][x.name.hour * 4 + x.name.minute//15]))
#        plt.clf()
#        plt.xticks(rotation=45, ha='right')
#        plt.plot([str(timedelta(hours = i//4, minutes = i%4 * 15)) for i in range(24*4)], significant_data[x["AP-MAC_x"]])
#        plt.xlim(7*4, 22*4)
#        plt.show()
        return 1-significant_data[x["AP-MAC_x"]][x.name.hour * 4 + x.name.minute//15]
#        return 1
    return 0

common = df.apply(seven_differencies, axis=1).sum()
total = df.apply(lambda x: 1, axis=1).sum()

print("Absolute score : "+str(common*15/60)+" ; total time : "+str(total*15/60)+"h ; relative score : "+str(100*common/total))
