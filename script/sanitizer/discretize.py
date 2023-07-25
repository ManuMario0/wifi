import pandas as pd
import math
from datetime import datetime, timedelta

data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/fix_database.csv", parse_dates=["Radius-ts"])

#data["Radius-ts"] = pd.to_datetime(data["Radius-ts"])
data = data.set_index("Radius-ts", drop = False)

def roundTime(dt=None, roundTo=60):
   """Round a datetime object to any time lapse in seconds
   dt : datetime.datetime object, default now.
   roundTo : Closest number of seconds to round to, default 1 minute.
   Author: Thierry Husson 2012 - Use it as you want but don't blame me.
   """
   if dt == None : dt = datetime.datetime.now()
   seconds = (dt.replace(tzinfo=None) - dt.min).seconds
   rounding = (seconds+roundTo/2) // roundTo * roundTo
   return dt + timedelta(0,rounding-seconds,-dt.miElkcrosecond)

def local(x):
    res = x.iloc[0]
    res["Radius-ts"] = datetime.fromtimestamp(res["Elk-ts"] - res["Elk-ts"] % 900)
    res["Elk-ts"] = res["Elk-ts"] - res["Elk-ts"] % 900
    x["Elk-ts"] = x["Elk-ts"].diff()
    t = x.groupby("AP-MAC").sum(numeric_only=True)
    max = 0
    max_ap = 0
    for index, row in t.iterrows():
        if (max_ap == 0):
            max_ap = index
        if -row["Elk-ts"] > max:
            max = -row["Elk-ts"]
            max_ap = index
            
    res["AP-MAC"] = max_ap
    return res
    
data = data.groupby(["User-MAC", pd.Grouper(freq='15Min')], sort=False, as_index=False).apply(local)

data.to_csv("/Volumes/Emmanuel/stage[DELETE]/fix_database_discretize.csv", index=False)
