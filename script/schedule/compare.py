import math, operator
from PIL import Image, ImageChops, ImageOps
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import math

def bar(x):
    if x > 220 :
        return 255
    else :
        return 0

def compare(file1, file2):
#    print(file1)
#    image1 = Image.open(file1)
#    image2 = Image.open(file2)
#    h1 = Image.eval(Image.open(file1).convert('L'), bar).convert('1')
#    h2 = Image.eval(Image.open(file2).convert('L'), bar).convert('1')
    h1 = Image.open(file1).convert('L')
    h2 = Image.open(file2).convert('L')
    
#    rrr = np.zeros((len(h1), len(h1[0])))
#    c1 = 0
#    c2 = 0
#    for i in range(len(h1)):
#        for j in range(len(h1[i])):
#            if (h1[i][j] > 100) and (h2[i][j] > 100):
#                rrr[i][j] = 1
#            if (h1[i][j] > 100):
#                c1 += 1
#            if (h2[i][j] > 100):
#                c2 += 1
    
#    invert = ImageOps.invert(ImageChops.difference(h1, h2))
    
    rrr = np.asarray(ImageChops.multiply(h1, h2).convert('L'))
    
#    c1 = np.sum(np.asarray(h1.convert('L')))
#    c2 = np.sum(np.asarray(h2.convert('L')))
    
    diff = []
    for x in rrr:
        diff.append(np.sum(x))
#    diff /= max(c1, c2)
#    diff = np.log(diff)
#    diff /= len(rrr)
    diff = max(diff)
    return diff
    
#    invert.save("diff.png")

#if __name__=='__main__':
#    import sys
#
#    file1, file2 = sys.argv[1:]
#    compare("../../uniform_qr/"+file1, "../../uniform_qr/"+file2)
#


import os

path = "../../uniform_qr/"
separator = "/"

years = os.listdir(path)
devices = []
for y in years:
    users = os.listdir(path+y)
    for u in users:
        _devices = os.listdir(path+y+separator+u)
        for d in _devices:
            devices.append([path+y+separator+u+separator+d, y+separator+u[:4]+separator+d[:4]])

dist = np.zeros((len(devices), len(devices)))
i = 0
for d1 in devices:
    j = 0
    for d2 in devices:
#        if compare(d1[0], d2[0]) > .05:
#            dist[i][j] = 1
        dist[i][j] = compare(d1[0], d2[0])
        j += 1
    i+=1
    
axis = ["" for i in devices]
i = 0
for d in devices:
    axis[i] = d[1]
    i+=1

fig, ax = plt.subplots(figsize = (30,30))

plt.yticks(np.arange(len(axis)), axis)
plt.xticks(np.arange(len(axis)), axis, rotation=90)

#plt.rc('xtick',labelsize=8)
#plt.rc('ytick',labelsize=8)

ax.tick_params(axis='x', labelsize=8)
ax.tick_params(axis='y', labelsize=8)

im = ax.imshow(dist, cmap="binary", norm=LogNorm())
fig.colorbar(im, ax=ax)
plt.savefig("matrix.png")
