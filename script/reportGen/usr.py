from datetime import datetime, timedelta
from typing import Any, Optional, List
import matplotlib.pyplot as plt
from matplotlib import colors
from dataclasses import dataclass
import numpy as np
from matplotlib.pyplot import figure
import math

from scipy.stats.kde import gaussian_kde

class Event():
    MOVING = 0
    STOP = 1
    
    def __init__(self, start: int, end: int, type: int, ap: int):
        self.start = start
        self.end = end
        self.type = type
        self.ap = ap
        self.duration = timedelta(seconds=end-start)
        
    def print(self):
        print("Start : "+str(self.start)+" ; End : "+str(self.end)+" ; type : "+str(self.type)+" ; AP : "+str(self.ap))
        
        
class Schedule():
    def __init__(self):
        self.events = []
        self.event_count = 0
        
    def add_event(self, e: Event) -> None:
        self.events.append(e)
        self.event_count += 1
        
    def _produce_color_histo(tbl: List[Any], title: str, n_bins: int, color: Optional[bool] = True) -> None:
        #fig, axs = plt.subplots(1, 1, tight_layout=True, figsize=(16, 6))
        plt.title(title)
        kde = gaussian_kde( tbl )
        
        dist_space = np.linspace( min(tbl) - 10, max(tbl) + 10, 200 )
        
        plt.plot(dist_space, kde(dist_space))
        
        #N, bins, patches = axs.hist(tbl, bins=n_bins, density=True)
        #plt.plot(dist_space, kde(dist_space))
        
#        if (color):
#            fracs = N / N.max()
#            norm = colors.Normalize(fracs.min(), fracs.max())
#
#            for thisfrac, thispatch in zip(fracs, patches):
#                color = plt.cm.viridis(norm(thisfrac))
#                thispatch.set_facecolor(color)
                
        day = ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"]
        
        ticks = [2*i for i in range(5*12)]
        label = ["" for i in range(5*12)]
        for i in range(len(label)):
            if (2*i%24 == 0):
                label[i] = day[2*i//24]
            else:
                label[i] = "{:0>2d}".format(2*i%24)
        plt.xticks(ticks, label, rotation=75)
        plt.xlim(0, 5*24)
        plt.xlabel('', fontsize=30)
        plt.tight_layout()
        
    def render_schedule(self, title: str, _week : datetime, usr) -> None:
        plt.close()
        fig = plt.figure(figsize=(18, 9))
        
        DAYS = ['Monday','Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday', 'Sunday']
        
        tbl_color = [name for name in colors.TABLEAU_COLORS]
        
        plt.title(title, y=1, fontsize=14)
        ax=fig.add_subplot(1, 1, 1)
        ax.set_xlim(-.5, len(DAYS) - 0.5)
        ax.set_xticks(range(len(DAYS)))
        ax.set_xticklabels(DAYS)
        ax.set_ylim(22, 7)
        ax.set_yticks(range(math.ceil(7), math.ceil(22)))
        ax.set_yticklabels(["{0}:00".format(h) for h in range(math.ceil(7), math.ceil(22))])
        ax.grid(axis='y', linestyle='--', linewidth=0.5)
        
        week = _week - timedelta(days=_week.weekday())
        
        i = 0
        while i < self.event_count and datetime.fromtimestamp(self.events[i].start) < week:
            i += 1
            
        while i < self.event_count and datetime.fromtimestamp(self.events[i].end) < week+timedelta(days=7):
            if self.events[i].type == Event.STOP and self.events[i].end - self.events[i].start < 6*3600:
                wd = datetime.fromtimestamp(self.events[i].start).weekday()
                x = [wd-.4, wd+.4]
                start = datetime.fromtimestamp(self.events[i].start)
                start = start.hour + start.minute / 60
                end = datetime.fromtimestamp(self.events[i].end)
                end = end.hour + end.minute / 60
                
                plt.fill_between(x, [start, start], [end, end], color=tbl_color[self.events[i].ap%len(tbl_color)])
                plt.text(wd, (start+end)/2, usr.ap[self.events[i].ap], ha='center', va='center', fontsize=10)
            i += 1
        
        
        
#        daily_schedule = [[] for i in range(7)]
#        local_schedule = [0 for i in range(self.event_count)]
#        for i in range(self.event_count):
#            e = self.events[i]
#            date = datetime.fromtimestamp(e.start)
#            delta = timedelta(days = date.weekday(), hours=date.hour, minutes=date.minute, seconds=date.second)
##            local_schedule.append(datetime(1970, 1, 1) + delta)
#            local_schedule[i] = delta.total_seconds()/3600
#            daily_schedule[int(delta.total_seconds())//(3600*24)].append(delta.total_seconds()/3600)
#
#        plt.close()
##        plt.clf()
#        figure(figsize=(16, 6))
##        Schedule._produce_color_histo(local_schedule, title, 300)
#        for i in range(5):
#            Schedule._produce_color_histo(daily_schedule[i], title, 300)
        
        
    def render_AD_schedule(self, title: str) -> None:
#        local_schedule = []
#        tmp = []
        daily_arrival = [[] for i in range(7)]
        daily_departure = [[] for i in range(7)]
        current_day = datetime.fromtimestamp(self.events[0].start)
        current_day =  ( current_day - timedelta(hours=current_day.hour, minutes=current_day.minute,seconds=current_day.second, microseconds=current_day.microsecond) )
        one_day = timedelta(days=1)
        last_visited = datetime.fromtimestamp(0)
        for i in range(self.event_count):
            e = self.events[i]
            date = datetime.fromtimestamp(e.start)
#            if (e.type == Event.MOVING):
            if (date > current_day + one_day):
                current_day = date
                current_day =  ( current_day - timedelta(hours=current_day.hour, minutes=current_day.minute,seconds=current_day.second, microseconds=current_day.microsecond) )
                if (last_visited != datetime.fromtimestamp(0)):
                    delta = timedelta(days = last_visited.weekday(), hours=last_visited.hour, minutes=last_visited.minute, seconds=last_visited.second)
#                    tmp.append(delta.total_seconds()/3600)
                    daily_departure[int(delta.total_seconds())//(3600*24)].append(delta.total_seconds()/3600)
                delta = timedelta(days = date.weekday(), hours=date.hour, minutes=date.minute, seconds=date.second)
#                local_schedule.append(delta.total_seconds()/3600)
                daily_arrival[int(delta.total_seconds())//(3600*24)].append(delta.total_seconds()/3600)
            last_visited = datetime.fromtimestamp(e.start)
            
        plt.close()
#        plt.clf()
        figure(figsize=(16, 6))
        for i in range(5):
            Schedule._produce_color_histo(daily_arrival[i], title, 300, color=False)
            Schedule._produce_color_histo(daily_departure[i], title, 300, color=False)
#        Schedule._produce_color_histo(local_schedule, title, 300, color=False)
#        Schedule._produce_color_histo(tmp, title, 300, color=False)

@dataclass
class AccessPoint:
    mac: str = ""
    visites: int = 0
        
class Calendar():
    def __init__(self):
        self.entries: List([datetime.datetime, Any]) = []
        self.entries_count = 0
        
    def add_entry(self, date: datetime, val: Any):
        self.entries.append([date, val])
        self.entries_count += 1
        

class Device():
    MOBILE = 0
    STATIC = 1
    UNKNOWN = 2
    
    
    def __init__(self, mac: str, type: int, schedule: Schedule, ap_freq: List[AccessPoint], ap_count: Calendar, ap_swap: Calendar):
        self.mac = mac
        self.type = type
        self.schedule = schedule
        self.ap_freq = ap_freq
        self.ap_count = ap_count
        self.ap_swap = ap_swap

class User():
    def __init__(self, uid: str, ap_count: int, ap: dict):
        self.uid = str
        self.ap_count = ap_count
        self.devices = []
        self.device_count = 0
        self.ap = ap
        self.types = [0 for i in range(3)]
        
    def add_device(self, device: Device):
        self.devices.append(device)
        self.device_count += 1
        self.types[device.type] += 1
        
    def _produce_bar_graph(x: List[Any], y: List[Any], title: str):
        plt.close()
        
        f, ax = plt.subplots(figsize=(10,6))
        plt.title(title)
        plt.bar(x, y)
        plt.xticks(rotation=80, rotation_mode = "anchor", horizontalalignment="right")
        plt.xlabel('', fontsize=30)
        plt.tight_layout()
        
    def get_next_stop(i, events):
        if (i == len(events)-1):
            return i
        i += 1
        while i < len(events) and events[i].type != Event.STOP:
            i+=1
        return i
        
    def get_favorite_transition(self) -> (str, str):
        transitions = [[0 for i in range(self.ap_count+1)] for j in range(self.ap_count+1)]
        start = 1674432000
#        start = 1661990400
        end = start+14*24*3600
        
        for d in self.devices:
            i = 0
            while (d.schedule.events[i].start < start):
                i+=1
            
            while (d.schedule.events[i].start < end and i+1 < d.schedule.event_count):
                if (d.schedule.events[i].type == Event.STOP):
                    j = User.get_next_stop(i, d.schedule.events)
                    if (d.schedule.events[i].ap != d.schedule.events[j].ap):
                        transitions[d.schedule.events[i].ap][d.schedule.events[j].ap] += 1
                i+=1
                
#        fig, ax = plt.subplots(figsize = (10,10))
#
#
#        im = ax.imshow(transitions)
#        fig.colorbar(im, ax=ax)
#        plt.show()
        
        return transitions
        
        
    def get_favorite_ap(self) -> str:
        ap_freq = [0 for i in range(self.ap_count+1)]
        total_time = 0
        for d in self.devices:
            if (d.type == Device.MOBILE or d.type == Device.UNKNOWN):
                for e in d.schedule.events:
                    delta = e.end - e.start
                    if (e.type == Event.STOP):
                        ap_freq[e.ap] += delta
            
        return self.ap[np.argmax(ap_freq)]
        
    def get_favorite_campus(self) -> str:
        campus = {"M" : 0, "A" : 1, "B" : 2, "C" : 3}
        ap_freq = [0 for i in range(len(campus))]
        for d in self.devices:
            if (d.type == Device.MOBILE or d.type == Device.UNKNOWN):
                for e in d.schedule.events:
                    delta = e.end - e.start
                    if (e.type == Event.STOP):
                        ap_freq[campus[self.ap[e.ap][0]]] += delta
                        
        return ["M", "A", "B", "C"][np.argmax(ap_freq)]
        
    def get_favorite_building(self) -> str:
        favorite_campus = self.get_favorite_campus()
        ap_freq = []
        buildings = []
        for d in self.devices:
            if (d.type == Device.MOBILE or d.type == Device.UNKNOWN):
                for e in d.schedule.events:
                    delta = e.end - e.start
                    if (e.type == Event.STOP and self.ap[e.ap][0] == favorite_campus):
                        building_name = User._get_building_name(self.ap[e.ap])
                        if building_name in buildings:
                            index = buildings.index(building_name)
                            ap_freq[index] += delta
                        else:
                            buildings.append(building_name)
                            ap_freq.append(delta)
                            
        return buildings[np.argmax(ap_freq)]
        
    def render_schedule_stats(self) -> None:
        events = [0 for d in self.devices]
        times = [0 for d in self.devices]
        i = 0
        for d in self.devices:
            s = d.schedule
            for e in s.events:
                if (e.type == Event.MOVING):
                    events[i] += e.end-e.start
                times[i] += e.end-e.start
            i += 1
            
        y = [0. for d in self.devices]
        for i in range(self.device_count):
            y[i] = events[i] / times[i]
            
        y.sort()
        plt.plot(y, '.')
        
        
    def render_ap_frequentation(self) -> None:
        ap_freq = [0 for i in range(self.ap_count)]
        aps_name = [v for k, v in self.ap.items()]
        total_time = 0
        for d in self.devices:
            if (d.type == Device.MOBILE or d.type == Device.UNKNOWN):
                for e in d.schedule.events:
                    delta = e.end - e.start
                    if (e.type == Event.STOP):
                        ap_freq[e.ap-1] += delta
                        total_time += delta
        
        decorated = [(ap_freq[i] / total_time, aps_name[i]) for i in range(self.ap_count)]
        decorated.sort(reverse = True)
        
        x = [x for y, x in decorated]
        y = [y for y, x in decorated]
        
        User._produce_bar_graph(x[:15], y[:15], "AP frequentations") # draw only first 15 aps
        
        
    def render_campus_frequentation(self) -> None:
        campus = {"M" : 0, "A" : 1, "B" : 2, "C" : 3}
        ap_freq = [0 for i in range(len(campus))]
        total_time = 0
        for d in self.devices:
            if (d.type == Device.MOBILE or d.type == Device.UNKNOWN):
                for e in d.schedule.events:
                    delta = e.end - e.start
                    if (e.type == Event.STOP):
                        ap_freq[campus[self.ap[e.ap][0]]] += delta
                        total_time += delta
        
        decorated = [(ap_freq[i] / total_time, ["Madrid", "Getafe", "Leganes", "Colmerajero"][i]) for i in range(len(campus))]
        
        x = [x for y, x in decorated]
        y = [y for y, x in decorated]
        
        User._produce_bar_graph(x, y, "Campus frequentations")


    def _get_building_name(_name: str) -> str:
        if (_name[1:3].isdigit()):
            return _name[1:3]
        else:
            name = []
            for i in _name:
                if (i.isdigit()):
                    break
                name.append(i)
            return ''.join(name)

    def render_building_frequentation(self) -> None:
        favorite_campus = self.get_favorite_campus()
        ap_freq = []
        buildings = []
        total_time = 0
        for d in self.devices:
            if (d.type == Device.MOBILE or d.type == Device.UNKNOWN):
                for e in d.schedule.events:
                    delta = e.end - e.start
                    if (e.type == Event.STOP and self.ap[e.ap][0] == favorite_campus):
                        building_name = User._get_building_name(self.ap[e.ap])
                        if building_name in buildings:
                            index = buildings.index(building_name)
                            ap_freq[index] += delta
                        else:
                            buildings.append(building_name)
                            ap_freq.append(delta)
                        total_time += delta
        
        decorated = [(buildings[i], ap_freq[i] / total_time) for i in range(len(buildings))]
        decorated.sort()
        
        x = [x for x, y in decorated]
        y = [y for x, y in decorated]
        
        User._produce_bar_graph(x, y, "Buildings frequentations")


    def _get_floor_name(_name: str) -> str:
        if (_name[1:3].isdigit()):
            return _name[3]
        else:
            i = 0
            while not _name[i].isdigit():
                i += 1
            return _name[i]
            
    def render_floor_frequentation(self) -> None:
        favorite_campus = self.get_favorite_campus()
        favorite_building = self.get_favorite_building()
        print(favorite_building)
        ap_freq = []
        floors = []
        total_time = 0
        for d in self.devices:
            if (d.type == Device.MOBILE or d.type == Device.UNKNOWN):
                for e in d.schedule.events:
                    delta = e.end - e.start
                    if (e.type == Event.STOP \
                        and self.ap[e.ap][0] == favorite_campus \
                        and User._get_building_name(self.ap[e.ap]) == favorite_building):
                        floor_name = User._get_floor_name(self.ap[e.ap])
                        if floor_name in floors:
                            index = floors.index(floor_name)
                            ap_freq[index] += delta
                        else:
                            floors.append(floor_name)
                            ap_freq.append(delta)
                        total_time += delta
        
        decorated = [(floors[i], ap_freq[i] / total_time) for i in range(len(floors))]
        decorated.sort()
        
        x = [x for x, y in decorated]
        y = [y for x, y in decorated]
        
        User._produce_bar_graph(x, y, "Buildings frequentations")
