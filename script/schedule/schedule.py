import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime, timedelta
import numpy as np
import os

data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/fix_database_discretize.csv", parse_dates=["Radius-ts"])
data = data.set_index("Radius-ts", drop = False)

devices_translation_data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/fix_devices.csv")

devices_translation = {}

def assign(x):
    devices_translation[x[0]] = x[1]

devices_translation_data[["BaseRadio MAC Address", "AP Name"]].apply(assign, axis = 1)

del devices_translation_data

ap_to_print = []

def count(x, ap_list):
    date = x["Radius-ts"]
    if x["AP-MAC"] not in devices_translation:
        return
    ap = devices_translation[x["AP-MAC"]]
    
    if ap not in ap_list:
        ap_list[ap] = [[0 for i in range(24*4)] for j in range(7)]
        
    timeline = ap_list[ap]
    timeline[date.weekday()][date.hour*4+date.minute//15] += 1
    ap_list[ap] = timeline
    
def hist(input):
    ap = {}
    
    input.apply(count, args=(ap,), axis = 1)
    
    sum = []
    aps = []
    v_max = 0
    for key, item in ap.items():
        sum.append(np.sum(item))
        for x in item:
            v_max = max(v_max, max(x))
        aps.append(key)
        
#    ap_to_print = []
#    hist = []
#    for i in range(5):
#        index = np.argmax(sum)
#        if (sum[index] > 0):
#            if aps[index] in devices_translation:
#                ap_to_print.append(devices_translation[aps[index]])
#                hist.append(ap[aps[index]])
#        sum[index] = 0

    hist = []
    ccount = 0
    for x in ap_to_print:
        if x in ap:
            hist.append(ap[x])
            ccount += 1
        else:
            hist.append([[0 for i in range(24*4)] for j in range(7)])
    
    dates = [timedelta(hours=i) for i in range(24)][7:22]
    
    days = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday"]
    
    if (len(ap_to_print) > 0 and len(sum) > 0 and max(sum) > 50 and ccount > 0):
        fig, ax = plt.subplots(ncols=5, sharey=True, figsize = (26,14))
        
        plt.yticks([4*i for i in range(len(dates))], dates)
        
        index = 0
        storage = []
        hh = np.array(hist)
        for x in ax:
            x.set_xticks(np.arange(len(ap_to_print)))
            x.set_xticklabels(ap_to_print, rotation=45, rotation_mode="anchor", ha="right")
#            x.set_xticks(np.arange(len(ap_to_print))-.5, minor=True)
#            x.tick_params(which="minor", bottom=False, left=False)
#            x.grid(which="minor", color="w", linestyle='-', linewidth=1)

            d = np.array(np.transpose([h[index][7*4:22*4] for h in hist]), dtype="float")
            if (d.max() > 0):
                d = d/d.max()
            for h in hist:
                u = h[index][7*4:22*4]
                if (hh.max() > 0):
                    u /= hh.max()
                u *= 255
                storage.append(u)
                
            im = x.imshow(d, vmin=0, vmax=1, aspect=1, cmap="binary")
            x.set_title(days[index])
            index += 1
        cbar = fig.colorbar(im, ax=ax)
#        cbar.ax.set_ylabel("Density", rotation=-90, va="bottom")
        
        path = "../../uniform_fp/"+str(custum_filter(input["Radius-ts"].iloc[0]))+"/"+input["User-ID"].iloc[0]+"/"
        name = path+input["User-MAC"].iloc[0]+".png"
        if not os.path.exists(path):
            os.makedirs(path)
#        plt.savefig(name)
#        plt.close()

        path = "../../uniform_qr/"+str(custum_filter(input["Radius-ts"].iloc[0]))+"/"+input["User-ID"].iloc[0]+"/"
        name = path+input["User-MAC"].iloc[0]+".png"
        if not os.path.exists(path):
            os.makedirs(path)
#        print(storage)
        st = np.array(storage)
        plt.imsave(name, st.astype(np.uint8), cmap="binary")
        plt.close()
#        plt.show()
        
        
def find_ap_to_print(x):
    if (len(ap_to_print) < 30 and x in devices_translation):
        ap_to_print.append(devices_translation[x])
        
data.groupby("AP-MAC")["Radius-ts"].count().sort_values(ascending=False).index.to_series().apply(find_ap_to_print)
        
        
        

def construct_year_hist(x):
    x.groupby("User-MAC").apply(hist)


def custum_filter(x):
    if x.month < 8 or (x.month == 8 and x.day < 15):
        return x.year - 1
    else:
        return x.year

data.groupby(custum_filter).apply(construct_year_hist)
