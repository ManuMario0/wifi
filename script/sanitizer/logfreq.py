import pandas as pd
import matplotlib.pyplot as plt
import math

import sys
sys.path[0:0] = ['../reportGen/']

import parser
import usr as user

usr = parser.parse("../../users/global.usr")

devices_translation_data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/fix_devices.csv")

devices_translation = {}

def assign(x):
    devices_translation[x[0]] = x[1]

devices_translation_data[["BaseRadio MAC Address", "AP Name"]].apply(assign, axis = 1)

del devices_translation_data

class Entry:
    TIMESTAMP = "Elk-ts"
    MAC = "User-MAC"
    APMAC = "AP-MAC"

data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/fix_database.csv")

hist = []

def aux(x):
    val = -x
    if not math.isnan(x) and val < 1800:
        hist.append(val/60)

per_ap_hist = [[[] for j in range(1000)] for i in range(1000)]

correspondance = []

tt = [[] for i in range(2)]

def aux2(x):
    t_start = x[0]
    t_end = x[1]
    ap_start = x[2]
    ap_end = x[3]
    
    if math.isnan(x[0]) or math.isnan(x[1]):
        return
    
    if ap_start not in correspondance:
        correspondance.append(ap_start)
        
    if ap_end not in correspondance:
        correspondance.append(ap_end)
    
    period = t_end - t_start
    
    if (period < 1800 and isinstance(ap_start, str)):
        if (correspondance.index(ap_start) == correspondance.index(ap_end)):
            tt[0].append(period/60)
        else:
            tt[1].append(period/60)
#        per_ap_hist[correspondance.index(ap_start)][correspondance.index(ap_end)].append(period/60)
        
        
per_device_hist = {}
for x in usr.devices:
    per_device_hist[x.mac] = []

def aux3(x):
    t_start = x[0]
    t_end = x[1]
    
    period = t_end - t_start
    
    if (period < 1800):
        if x[2] in per_device_hist:
            per_device_hist[x[2]].append(period/60)
    

def compute_histo(x):
#    print(x)
    x[Entry.TIMESTAMP].diff().apply(aux)
    x["Timestamp shift"] = x[Entry.TIMESTAMP].shift(fill_value=10000000000000000)
    x["APMAC shift"] = x[Entry.APMAC].shift(fill_value="")
    x[[Entry.TIMESTAMP, "Timestamp shift", Entry.APMAC, "APMAC shift"]].apply(aux2, axis = 1)
#    x[[Entry.TIMESTAMP, "Timestamp shift", Entry.MAC]].apply(aux3, axis = 1)

data.groupby(Entry.MAC).apply(compute_histo)

#plt.hist(hist, bins=2000)
#plt.show()
#
#i = 0
#j = 0
#for a in per_ap_hist:
#    for b in a:
#        if (len(b) > 1000):
#            plt.clf()
#            plt.figure(figsize=(16, 6))
#            plt.hist(b, bins=250)
#            plt.xlim(-2, 22)
#            if isinstance(correspondance[i], str) and isinstance(correspondance[j], str):
#                plt.savefig("../../aphist/ap-"+devices_translation[correspondance[i]]+"-"+devices_translation[correspondance[j]]+".png")
#            else:
#                plt.savefig("../../aphist/ap-"+str(i)+"-"+str(j)+".png")
#            plt.close()
#        j += 1
#    j = 0
#    i += 1

plt.clf()
plt.figure(figsize=(16, 6))
plt.hist(tt[0], bins=250)
plt.xlim(-2, 22)
plt.savefig("../../aphist/common.png")
plt.close()

plt.clf()
plt.figure(figsize=(16, 6))
plt.hist(tt[1], bins=250)
plt.xlim(-2, 22)
plt.savefig("../../aphist/diff.png")
plt.close()

#for x in usr.devices:
#    plt.figure(figsize=(16, 6))
#    plt.title(x.mac)
#    plt.hist(per_device_hist[x.mac], bins=250)
#    plt.xlim(-2, 22)
##    plt.show()
#    if x.type == user.Device.MOBILE:
#        plt.savefig("../../aphist/mobile/"+x.mac)
#    else:
#        plt.savefig("../../aphist/static/"+x.mac)
#    plt.close()
