import pandas as pd

data = pd.read_csv("/Volumes/Emmanuel/stage[DELETE]/fix_database.csv")

def local(x):
    if (x["User-MAC"].iloc[0] == "c92e359744197fb1853681fca8e4645ba5cb96c578e88ebcd234087ea112b5fb"):
        global res
        res = x

data.groupby("User-MAC").apply(local)

res.to_csv("bloubloub.csv")
