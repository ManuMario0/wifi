import matplotlib.pyplot as plt
from matplotlib import colors
from matplotlib.ticker import PercentFormatter
import sys
import calendar
import os
import math
import random
import parser
import tex
import usr
from datetime import datetime, timedelta
import numpy as np

plt.style.use('tableau-colorblind10')
filename = sys.argv[1]

prefix = ''.join(["../../reports/log/", filename[0:-4], "/"])
if not os.path.exists(prefix):
    os.makedirs(prefix)

usr = parser.parse("../../users/"+filename)

usr.devices[4].schedule.render_schedule("2022-10-03", datetime.strptime("2022-10-03", "%Y-%m-%d"), usr)
plt.savefig("marcus/2022-10-03.png")
exit()
#usr = parser.parse("../../users/eea8691e6d1c98893e2050a4a7b8653df724dceb66bfca535ba2bfaa0e9547e1.usr")
##usr = parser.parse("../../users/63037c3a59b5cba3498e93d31a662f3859d06e67e8d651c422539056fd3f3bd7.usr")
#
#transitions = usr.get_favorite_transition()
#
#def get_max_index():
#    max = 0
#    i = 0
#    j = 0
#    for x in range(len(transitions)):
#        for y in range(len(transitions[x])):
#            if (transitions[x][y] > max):
#                i = x
#                j = y
#                max = transitions[x][y]
#
#    return (i, j)
#
#
#for _ in range(10):
#    i, j = get_max_index()
#    print(usr.ap[i]+"     "+usr.ap[j])
#    transitions[i][j] = 0

#usr.render_schedule_stats()
##plt.yscale('log')
#plt.show()
#
#usr.devices[1].schedule.render_schedule("test")
#plt.show()
#
#usr.devices[0].schedule.render_AD_schedule("test")
#
#usr.render_ap_frequentations()
#plt.show()

#usr.render_floor_frequentation()
#plt.show()

#
#def fft_fingerprint(d, start: datetime, end: datetime) -> None:
#    start_ts = int(datetime.timestamp(start))
#    end_ts = int(datetime.timestamp(end))
#
#    values = [0 for x in range(end_ts - start_ts)]
#
#    index = 0
#    while (index < d.schedule.event_count and d.schedule.events[index].start < start_ts):
#        index += 1
#
#    e = d.schedule.events[index]
#    while (e.end < end_ts):
#        if (e.type == 1):
#            for i in range(e.start, e.end):
#                values[i - start_ts] = e.ap
#        index += 1
#        e = d.schedule.events[index]
#
#    res = np.fft.rfft(values)
#    freqs = np.fft.rfftfreq(len(values))
#    return np.abs(res)/len(values), freqs
#
#start_w = datetime.strptime("2022-09-12", "%Y-%m-%d")
#end_w = datetime.strptime("2022-09-17", "%Y-%m-%d")
#
#printer = [[0, 0] for i in range(216001)]
#
#for i in range(5):
#    res, _ = fft_fingerprint(usr.devices[0], start_w, end_w)
#    for j in range(216001):
#        printer[j][0] += res[j]/5
#
#    start_w += timedelta(days = 7)
#    end_w += timedelta(days = 7)
#
#usr = parser.parse("../../users/eea8691e6d1c98893e2050a4a7b8653df724dceb66bfca535ba2bfaa0e9547e1.usr")
#
#for i in range(5):
#    res, _ = fft_fingerprint(usr.devices[1], start_w, end_w)
#    for j in range(216001):
#        printer[j][1] += res[j]/5
#
#    start_w += timedelta(days = 7)
#    end_w += timedelta(days = 7)
#
#
#plt.plot(printer)
#plt.show()

#usr = parser.parse("../../users/eea8691e6d1c98893e2050a4a7b8653df724dceb66bfca535ba2bfaa0e9547e1.usr")
#usr = parser.parse("../../users/63037c3a59b5cba3498e93d31a662f3859d06e67e8d651c422539056fd3f3bd7.usr")
#usr = parser.parse("../../users/0f0e41bc7c61f414ae324f321e1454efe99fc11c9eef0bf7fea3f8916f1df564.usr")
#
#res2, freqs2 = fft_fingerprint(usr.devices[1], datetime.strptime("2022-05-16", "%Y-%m-%d"), datetime.strptime("2022-05-21", "%Y-%m-%d"))
#
#printer = [[res1[i], res2[i]] for i in range(min(len(res1), len(res2)))]
#freqs = [[freqs1[i], freqs2[i]] for i in range(min(len(freqs1), len(freqs2)))]
#
#diff = [abs(res1[i] - res2[i]) for i in range(min(len(res1), len(res2)))]
#
#print(np.sum(diff[:10]))
#
#plt.plot(printer)
##plt.xlim(-0.00003, .0001)
#plt.show()

#res1, freqs1 = fft_fingerprint(usr.devices[0], datetime.strptime("2022-09-05", "%Y-%m-%d"), datetime.strptime("2022-09-10", "%Y-%m-%d"))
#
##usr = parser.parse("../../users/eea8691e6d1c98893e2050a4a7b8653df724dceb66bfca535ba2bfaa0e9547e1.usr")
#
#res2, freqs2 = fft_fingerprint(usr.devices[0], datetime.strptime("2022-09-12", "%Y-%m-%d"), datetime.strptime("2022-09-17", "%Y-%m-%d"))
#
#printer = [(res1[i] - res2[i])/(2*min(len(res1), len(res2))) for i in range(min(len(res1), len(res2)))]
#
#print(np.sum(printer))
#
#plt.plot(printer)
##plt.xlim(0, .0001)
#plt.show()

#res, freqs = fft_fingerprint(usr.devices[0], datetime.strptime("2022-09-05", "%Y-%m-%d"), datetime.strptime("2022-09-10", "%Y-%m-%d"))
#plt.plot(freqs, res)
#plt.xlim(0, 0.0001)
#plt.show()

figs = tex.build_figures(prefix, usr)

_filename = list(filename)
_filename[-3:] = ['t', 'e', 'x']
tex.write_latex_file(prefix+''.join(_filename), usr, figs)

tex.build_latex_file(''.join(_filename), prefix)

pdf = "../../reports/pdf/"+sys.argv[2]+"/"+"common"+"/"
if not os.path.exists(pdf):
    os.makedirs(pdf)
_filename[-3:] = ['p', 'd', 'f']
os.system("cp "+prefix+''.join(_filename)+" "+pdf)

symlink = "../../reports/pdf/"+sys.argv[2]+"/"+usr.get_favorite_ap()+"/"
if not os.path.exists(symlink):
    os.makedirs(symlink)
_filename[-3:] = ['p', 'd', 'f']
os.system("ln -s "+"../common/"+''.join(_filename)+" "+symlink)

#
#uid = ""
#device_count = 0
#statics = []
#mobiles = []
#aps_name = []
#
#types = {"MOVING" : 0, "STOP" : 1}
#device_type = {"MOBILE" : 0, "STATIC" : 1, "UNKNOWN" : 2}
#campus = {"M" : 0, "A" : 1, "B" : 2, "C" : 3}
#rv_device_type = {0 : "MOBILE", 1 : "STATIC"}
#
#overall_freq = 0
#
#ap_count_average_data = []
#ap_swap_average_data = []
#
#
#
#with open("../../users/"+filename, "r") as f:
#    data = f.read()
#    lines = data.split('\n')
#    uid=lines[0].split(':')[1]
#    device_count=int(lines[1].split(':')[1])
#    i=5
#
#    for j in range(device_count):
#        mac = lines[i].split(':')[1]
#        i+=1
#        type = device_type[lines[i].split(':')[1]]
#        i+=2
#        schedule = []
#        while (lines[i] != "end"):
#            current_line = lines[i].split(';')
#            e_start = int(current_line[0].split(':')[1])
#            e_end = int(current_line[1].split(':')[1])
#            e_type = types[current_line[2].split(':')[1]]
#            e_ap = int(current_line[3].split(':')[1])
#            schedule.append([e_start, e_end, e_type, e_ap])
#            i+=1
#        i+=2
#        aps = []
#        total_freq = 0
#        while (lines[i] != "end"):
#            current_line = lines[i].split(';')
#            apuid = current_line[0].split(':')[1]
#            freq = int(current_line[1].split(':')[1])
#            total_freq += freq
#            aps.append([apuid, freq])
#            if (j == 0):
#                aps_name.append(apuid)
#            i+=1
#        i+=2
#        day_ap = [0 for k in range(365)]
#        ap_sum = []
#        while (lines[i] != "end"):
#            current_line = lines[i].split(';')
#            date = datetime.fromisoformat(current_line[0].split(':')[1])
#            ap_count = int(current_line[1].split(':')[1])
#            deltaTime = max(aps[e][1] for e in range(953))
#            #for k in range(math.ceil(100.*ap_count/deltaTime)):
#            ap_sum.append(ap_count)
#            for k in range(ap_count):
#                day_ap[k] += 1
#            i+=1
#        i+=2
#        ap_swaps = [0 for k in range(2000)]
#        ap_swap_sum = []
#        while (lines[i] != "end"):
#            current_line = lines[i].split(';')
#            date = datetime.fromisoformat(current_line[0].split(':')[1])
#            ap_count = int(current_line[1].split(':')[1])
#            deltaTime = max(aps[e][1] for e in range(953))
#            #for k in range(math.ceil(100.*ap_count/deltaTime)):
#            ap_swap_sum.append(ap_count)
#            for k in range(ap_count):
#                    ap_swaps[k] += 1
#            i+=1
#        #if (ap_index > 0 and ap_swap_index > 0):
#        ap_count_average_data.append(np.percentile(ap_sum, 80))
#        ap_swap_average_data.append(np.percentile(ap_swap_sum, 80))
#        if (type == 0):
#            mobiles.append([mac, type, total_freq, aps, schedule, day_ap, ap_swaps])
#        elif (type == 1):
#            statics.append([mac, type, total_freq, aps, schedule, day_ap, ap_swaps])
#        overall_freq += total_freq
#        i+=3
#        if (i>=len(lines)):
#            break
#
#axe = []
#legend = []
#for i in range(953):
#    tmp = []
#    for d in mobiles:
#        tmp.append(d[3][i][1] / d[2])
#    for d in statics:
#        tmp.append(d[3][i][1] / d[2])
#    axe.append(tmp)
#
#for d in mobiles:
#    legend.append("MOBILE")
#for d in statics:
#    legend.append("STATIC")
#
#plt.title("Per device AP frequentation")
#axe.sort(reverse=True)
#empty = [0. for i in range(device_count)]
#plt.plot([x for x in axe if x != empty])
#plt.legend(legend)
##plt.show()
#plt.savefig(''.join([prefix, "devices_ap.png"]))
#
#plt.clf()
#
#overall_ap_stats = [0 for i in range(953)]
#campus_stats = [0 for i in range(4)]
#total_time = 0
#for d in mobiles:
#    for x in d[4]:
#        if (x[3] != -1 and x[1] - x[0] < 6*3600):
#            overall_ap_stats[x[3]] += x[1] - x[0]
#            campus_stats[campus[d[3][x[3]][0][0]]] += (x[1] - x[0])
#            total_time += overall_ap_stats[x[3]]
#
##for i in range(953):
##    for d in mobiles:
##        overall_ap_stats[i] += d[3][i][1]
##        campus_stats[campus[d[3][i][0][0]]]+=d[3][i][1]/overall_freq
##    for d in statics:
##        overall_ap_stats[i] += d[3][i][1]
##        campus_stats[campus[d[3][i][0][0]]]+=d[3][i][1]/overall_freq
#
#decorated = [(overall_ap_stats[i]/total_time, aps_name[i]) for i in range(953)]
#decorated.sort(reverse = True)
#
#x = [x for y, x in decorated if y > 0.0001]
#y = [y for y, x in decorated if y > 0.0001]
#
#plt.title("AP frequentation")
#plt.xticks(rotation=90)
#plt.bar(x, y)
#plt.tight_layout()
#plt.savefig(''.join([prefix, "ap_freq.png"]))
#plt.show()
#
#plt.clf()
#
#ap_stats = [[0 for j in range(device_count)] for i in range(110)] # we suppose we limite our selves at one year of data
#for i in range(953):
#    index_device = 0
#    for d in mobiles:
#        count = sum(1 for e in range(953) if d[3][e][1])
#        deltaTime = max(d[3][e][1] for e in range(953))
#        if (deltaTime > 20):
#            for j in range(math.ceil(100.*d[3][i][1]/deltaTime)):
#                ap_stats[j][index_device]+=1/count
#            index_device += 1
#
#    for d in statics:
#        count = sum(1 for e in range(953) if d[3][e][1])
#        deltaTime = max(d[3][e][1] for e in range(953))
#        if (deltaTime > 20):
#            for j in range(math.ceil(100.*d[3][i][1]/deltaTime)):
#                ap_stats[j][index_device]+=1/count
#            index_device += 1
#
#plt.title("Number of AP encountered in more than n day")
#plt.plot([i+1 for i in range(110)], ap_stats)
##plt.legend(legend)
#plt.savefig(''.join([prefix, "ap_count.png"]))
#
#plt.clf()
#
#plt.title("Campus frequentation")
#plt.xticks(rotation=90)
#plt.bar(["Madrid", "Getafe", "Leganes", "Colmerajero"], campus_stats)
#plt.tight_layout()
#plt.savefig(''.join([prefix, "campus_freq.png"]))
#
#plt.clf()
#
#prefered_campus = np.argmax(campus_stats)
#buildings_stats = [0 for i in range(25)] #ruffly the max number of building per campus, just lazy to check
#for i in range(953):
#    for d in mobiles:
#        if (campus[d[3][i][0][0]] == prefered_campus and list(d[3][i][0])[1].isdigit() and list(d[3][i][0])[2].isdigit()):
#            buildings_stats[int(''.join(d[3][i][0][1:3]))]+=d[3][i][1]
##    for d in statics:
##        if (campus[d[3][i][0][0]] == prefered_campus and d[3][i][0][1:2].isdigit()):
##            buildings_stats[int(''.join(d[3][i][0][1:3]))]+=d[3][i][1]
#
#decorated = [(buildings_stats[i], i) for i in range(25)]
#decorated.sort(reverse = True)
#
#x = [x for y, x in decorated if y>0]
#y = [y/overall_freq for y, x in decorated if y>0]
#
#plt.title("Building frequentation on the favorite campus")
#plt.bar(x, y)
#plt.xticks(x)
#plt.tight_layout()
#plt.savefig(''.join([prefix, "building_freq.png"]))
#
#plt.clf()
#
#
#
#prefered_building = np.argmax(buildings_stats)
#floors_stats = [0 for i in range(7)] #ruffly the max number of floors per building, just lazy to check
#for i in range(953):
#    for d in mobiles:
#        if (campus[d[3][i][0][0]] == prefered_campus and list(d[3][i][0])[1].isdigit() and list(d[3][i][0])[2].isdigit() and int(''.join(d[3][i][0][1:3])) == prefered_building):
#            if d[3][i][0][3] == 'S' or d[3][i][0][3] == 'B':
#                floors_stats[-1]+=d[3][i][1]
#            else:
#                floors_stats[int(d[3][i][0][3])]+=d[3][i][1]
##    for d in statics:
##        if (campus[d[3][i][0][0]] == prefered_campus and list(d[3][i][0])[1].isdigit() and int(''.join(d[3][i][0][1:3])) == prefered_building):
##            if (d[3][i][0][3]) == 'S':
##                floors_stats[-1]+=d[3][i][1]
##            else:
##                floors_stats[int(d[3][i][0][3])]+=d[3][i][1]
#
#decorated = [(floors_stats[i], i) for i in range(7)]
#decorated[6] = (floors_stats[6], -1)
#decorated.sort(reverse = True)
#
#x = [x for y, x in decorated if y>0]
#y = [y/overall_freq for y, x in decorated if y>0]
#
#plt.title("Floor frequentation in the favorite building")
#plt.bar(x, y)
#plt.xticks(x)
#plt.tight_layout()
#plt.savefig(''.join([prefix, "floor_freq.png"]))
#
#plt.clf()
#
#
#ap_diff_histo = [[] for i in range(100)]
#for i in range(100):
#    for d in mobiles:
#        ap_diff_histo[i].append(d[5][i]/max(d[5]))
#        #ap_diff_histo[i].append(d[5][i])
#    for d in statics:
#        ap_diff_histo[i].append(d[5][i]/max(d[5]))
#        #ap_diff_histo[i].append(d[5][i])
#
#plt.title("Daily AP histogram")
#plt.plot(ap_diff_histo)
#plt.tight_layout()
##plt.legend(legend)
##plt.show()
#plt.savefig(''.join([prefix, "daily_ap_histo.png"]))
#
#plt.clf()
#
#
#ap_diff_histo = [[] for i in range(200)]
#for i in range(200):
#    for d in mobiles:
#        ap_diff_histo[i].append(d[6][i]/max(d[6]))
#        #ap_diff_histo[i].append(d[6][i])
#    for d in statics:
#        ap_diff_histo[i].append(d[6][i]/max(d[6]))
#        #ap_diff_histo[i].append(d[6][i])
#
#plt.title("AP swap histogram")
#plt.plot(ap_diff_histo)
#plt.tight_layout()
##plt.legend(legend)
#plt.savefig(''.join([prefix, "ap_swap_histo.png"]))
##plt.show()
#
#plt.clf()
#
#
#i = 0
#schedule = []
#for d in mobiles:
#    local_schedule = []
#    for s in d[4]:
#        date = datetime.fromtimestamp(s[0])
#        delta = timedelta(days = date.weekday(), hours=date.hour, minutes=date.minute, seconds=date.second)
#        local_schedule.append(datetime(1970, 1, 1) + delta)
#    # plt.title("Schedule histo")
#    #plt.hist(local_schedule, bins=400)
#    n_bins = 200
#
#    fig, axs = plt.subplots(1, 1, tight_layout=True)
#    plt.title("Schedule histogram of mobile "+str(i))
#
#    # N is the count in each bin, bins is the lower-limit of the bin
#    N, bins, patches = axs.hist(local_schedule, bins=n_bins)
#
#    # We'll color code by height, but you could use any scalar
#    fracs = N / N.max()
#
#    # we need to normalize the data to 0..1 for the full range of the colormap
#    norm = colors.Normalize(fracs.min(), fracs.max())
#
#    # Now, we'll loop through our objects and set the color of each accordingly
#    for thisfrac, thispatch in zip(fracs, patches):
#        color = plt.cm.viridis(norm(thisfrac))
#        thispatch.set_facecolor(color)
#
#    # We can also normalize our inputs by the total number of counts
#    #axs.hist(local_schedule, bins=n_bins, density=True)
#
#    # Now we format the y-axis to display percentage
#    # axs.yaxis.set_major_formatter(PercentFormatter(xmax=1))
#
#    # plt.tight_layout()
#    #plt.legend(legend)
#    plt.savefig(''.join([prefix, "schedule_"+str(i)+".png"]))
#    i+=1
#    #plt.show()
#
#    axs.remove()
#    plt.clf()
#    plt.close()
#
##i = 0
##for d in mobiles:
##    local_schedule = []
##    current_day = datetime.fromtimestamp(d[4][0][0])
##    current_day =  ( current_day - timedelta(hours=current_day.hour, minutes=current_day.minute,seconds=current_day.second, microseconds=current_day.microsecond) )
##    one_day = timedelta(days=1)
##    last_visited = datetime.fromtimestamp(0)
##    for s in d[4]:
##        date = datetime.fromtimestamp(s[0])
##        if (date > current_day + one_day):
##            current_day = date
##            current_day =  ( current_day - timedelta(hours=current_day.hour, minutes=current_day.minute,seconds=current_day.second, microseconds=current_day.microsecond) )
##            if (last_visited != datetime.fromtimestamp(0)):
##                delta = timedelta(days = last_visited.weekday(), hours=last_visited.hour, minutes=last_visited.minute, seconds=last_visited.second)
##                local_schedule.append(datetime(1970, 1, 1) + delta)
##            delta = timedelta(days = date.weekday(), hours=date.hour, minutes=date.minute, seconds=date.second)
##            local_schedule.append(datetime(1970, 1, 1) + delta)
##        last_visited = date
###        delta = timedelta(days = date.weekday(), hours=date.hour, minutes=date.minute, seconds=date.second)
###        local_schedule.append(datetime(1970, 1, 1) + delta)
##    # plt.title("Schedule histo")
##    #plt.hist(local_schedule, bins=400)
##    n_bins = 600
##
##    fig, axs = plt.subplots(1, 1, tight_layout=True)
##    plt.title("Schedule histogram of mobile "+str(i))
##
##    # N is the count in each bin, bins is the lower-limit of the bin
##    N, bins, patches = axs.hist(local_schedule, bins=n_bins)
##
##    # We'll color code by height, but you could use any scalar
##    fracs = N / N.max()
##
##    # we need to normalize the data to 0..1 for the full range of the colormap
##    norm = colors.Normalize(fracs.min(), fracs.max())
##
##    # Now, we'll loop through our objects and set the color of each accordingly
##    for thisfrac, thispatch in zip(fracs, patches):
##        color = plt.cm.viridis(norm(thisfrac))
##        thispatch.set_facecolor(color)
##
##    # We can also normalize our inputs by the total number of counts
##    #axs.hist(local_schedule, bins=n_bins, density=True)
##
##    # Now we format the y-axis to display percentage
##    # axs.yaxis.set_major_formatter(PercentFormatter(xmax=1))
##
##    # plt.tight_layout()
##    #plt.legend(legend)
##    #plt.savefig(''.join([prefix, "schedule_"+str(i)+".png"]))
##    i+=1
##    #plt.show()
##
##    axs.remove()
##    plt.clf()
##    plt.close()
#
#
#plt.title("AP swap/AP count")
#plt.scatter(ap_count_average_data, ap_swap_average_data)
##plt.xscale("log")
#plt.xlim(0, 120)
#plt.ylim(0, 120)
##plt.tight_layout()
##plt.legend(legend)
##plt.savefig(''.join([prefix, "ap_swap_histo.png"]))
##plt.show()
#
#plt.clf()
#
#max_ap = []
#for d in mobiles:
#    tmp = [0 for i in range(1000)]
#    for x in d[4]:
#        if (x[3] != -1 and x[1] - x[0] < 6*3600):
#            tmp[x[3]] += x[1] - x[0]
#    #max_ap.append(max(d[3], key=(lambda x:x[1]))[1]/d[2])
#    if (np.sum(tmp) > 0):
#        max_ap.append(max(tmp)/np.sum(tmp))
#for d in statics:
#    tmp = [0 for i in range(1000)]
#    for x in d[4]:
#        if (x[3] != -1 and x[1] - x[0] < 6*3600):
#            tmp[x[3]] += x[1] - x[0]
#    #max_ap.append(max(d[3], key=(lambda x:x[1]))[1]/d[2])
#    if (np.sum(tmp) > 0):
#        max_ap.append(max(tmp)/np.sum(tmp))
#
##print(max_ap)
#max_ap.sort()
#plt.plot(max_ap, '.')
#plt.show()
#
#moving_count = []
#for d in mobiles:
#    tmp_sum = 0
#    tmp = 0
#    for x in d[4]:
#        if (x[2] == 0 and x[1] - x[0] < 3*3600):
#           tmp_sum += 1
##        if (x[3] != -1 and x[1] - x[0] < 6*3600):
#        if (x[1] - x[0] < 3*3600):
#            tmp += x[1] - x[0]
#    #max_ap.append(max(d[3], key=(lambda x:x[1]))[1]/d[2])
##    print(str(tmp_sum) + ";" + str(tmp))
#    if (tmp > 0):
#        moving_count.append(3600*tmp_sum/tmp)
#for d in statics:
#    tmp_sum = 0
#    tmp = 0
#    for x in d[4]:
#        if (x[2] == 0 and x[1] - x[0] < 3*3600):
#           tmp_sum+=1
##        if (x[3] != -1 and x[1] - x[0] < 6*3600):
#        if (x[1] - x[0] < 3*3600):
#            tmp += x[1] - x[0]
#    #max_ap.append(max(d[3], key=(lambda x:x[1]))[1]/d[2])
#    if (tmp > 0):
#        moving_count.append(3600*tmp_sum/tmp)
#
##print(max_ap)
#moving_count.sort()
#plt.plot(moving_count, '.')
##plt.yscale("log")
#plt.show()
#
##color_dico = ["PastelRed", "PastelOrange", "PastelYellow", "PastelGreen", "PastelCyan", "PastelBlue", "PastelPurple", "PastelGray"]
