from usr import *
from datetime import datetime, timedelta
import matplotlib.pyplot as plt
import os
import random
from typing import io, Optional, IO, List

color_dico = ["AliceBlue",
"Aqua",
"Aquamarine",
"Beige",
"Bisque",
"BlanchedAlmond",
"Blue",
"BlueViolet",
"Brown",
"BurlyWood",
"CadetBlue",
"Chartreuse",
"Chocolate",
"Coral",
"CornflowerBlue",
"Cornsilk",
"Crimson",
"Cyan",
"DarkBlue",
"DarkCyan",
"DarkGoldenrod",
"DarkGray",
"DarkGreen",
"DarkGrey",
"DarkKhaki",
"DarkMagenta",
"DarkOliveGreen",
"DarkOrange",
"DarkOrchid",
"DarkRed",
"DarkSalmon",
"DarkSeaGreen",
"DarkSlateBlue",
"DarkSlateGray",
"DarkTurquoise",
"DarkViolet",
"DeepPink",
"DeepSkyBlue",
"DimGray",
"DodgerBlue",
"FireBrick",
"FloralWhite",
"ForestGreen",
"Fuchsia",
"Gainsboro",
"GhostWhite",
"Gold",
"Goldenrod",
"Gray",
"Green",
"GreenYellow",
#"Honeydew",
"HotPink",
"IndianRed",
"Indigo",
"Ivory",
"Khaki",
"Lavender",
"LavenderBlush",
"LawnGreen",
"LemonChiffon",
"LightBlue",
"LightCoral",
"LightCyan",
"LightGoldenrodYellow",
"LightGray",
"LightGreen",
"LightGrey",
"LightPink",
"LightSalmon",
"LightSeaGreen",
"LightSkyBlue",
"LightSlateGray",
"LightSteelBlue",
"LightYellow",
"Lime",
"LimeGreen",
"Linen",
"Magenta",
"Maroon",
"MediumAquamarine",
"MediumBlue",
"MediumOrchid",
"MediumPurple",
"MediumSeaGreen",
"MediumSlateBlue",
"MediumSpringGreen",
"MediumTurquoise",
"MediumVioletRed",
"MidnightBlue",
"MintCream",
"MistyRose",
"Moccasin",
"NavajoWhite",
"Navy",
"OldLace",
"Olive",
"OliveDrab",
"Orange",
"OrangeRed",
"Orchid",
"PaleGoldenrod",
"PaleGreen",
"PaleTurquoise",
"PaleVioletRed",
"PapayaWhip",
"PeachPuff",
"Peru",
"Pink",
"Plum",
"PowderBlue",
"Purple",
"Red",
"RosyBrown",
"RoyalBlue",
"SaddleBrown",
"Salmon",
"SandyBrown",
"SeaGreen",
"Seashell",
"Sienna",
"Silver",
"SkyBlue",
"SlateBlue",
"SlateGray",
"Snow",
"SpringGreen",
"SteelBlue",
"Tan",
"Teal",
"Thistle",
"Tomato",
"Turquoise",
"Violet",
"Wheat",
"WhiteSmoke",
"Yellow",
"YellowGreen"]

random.shuffle(color_dico)

intro = "\\documentclass[a4paper, oneside]{article}\n\
\\usepackage[utf8]{inputenc}\n\
\\usepackage[english]{babel}\n\
\\usepackage[T1]{fontenc}\n\
\\usepackage{amsmath}\n\
\\usepackage{amsfonts}\n\
\\usepackage{amssymb}\n\
\\usepackage{amsthm}\n\
\\usepackage{hyperref}\n\
\\usepackage{graphicx}\n\
\\usepackage[svgnames]{xcolor}\
\\usepackage[margin={2cm, 2cm}]{geometry}\n\
\\usepackage{../../reports/tex/moodtracker}\n\
\\author{Emmanuel MÉRA}\n"


class Figure():
    FULL = 0
    HALF = 1
    
    def __init__(self, name: str, scale: int, desc: Optional[str] = None):
        self.name = name
        self.scale = scale
        self.desc = desc
        
    def get_tex_line(self) -> str:
        if self.desc is None:
            if (self.scale == Figure.FULL):
                return "\n\\includegraphics[width=1.\linewidth]{"+self.name+"}\n"
            else:
                return "\\includegraphics[width=.5\linewidth]{"+self.name+"}\n"
        else:
            if (self.scale == FULL):
                return desc+"\n\\includegraphics[width=1.\linewidth]{"+self.name+"}\n"
            else:
                return desc+"\n\\includegraphics[width=.5\linewidth]{"+self.name+"}\n"

def write_intro(f: IO, usr: User) -> List[str]:
    ap_encounter = []
    i = 0
    
    for d in usr.devices:
        if (d.type == Device.MOBILE):
            for e in d.schedule.events:
                if (e.type == Event.STOP):
                    if e.ap not in ap_encounter:
                        f.write("\\newmood["+usr.ap[e.ap]+"]{"+str(i)+"}{"+color_dico[i]+"}\n")
                        ap_encounter.append(e.ap)
                        i += 1
                    
    return ap_encounter

def write_schedule(f: IO, usr: User, start: datetime, end: datetime) -> None:
    start_ts = datetime.timestamp(start)
    end_ts = datetime.timestamp(end)
    ap_encounter = []
    intro = []
    text = []
    
    indexes = [0 for i in range(usr.types[Device.MOBILE])]
    
    i = 0
    for d in usr.devices:
        if (d.type == Device.MOBILE):
            while (indexes[i] < d.schedule.event_count and d.schedule.events[indexes[i]].start < start_ts):
                indexes[i]+=1
            i+=1
        
    while (start_ts < end_ts):
        gothrough = [0 for i in range(usr.types[Device.MOBILE])]
        holding_time = [0 for x in range(len(usr.ap)+1)]
        i = 0
        for d in usr.devices:
            if (d.type == Device.MOBILE):
                while (indexes[i] < d.schedule.event_count and d.schedule.events[indexes[i]].start < start_ts+86400):
                    e = d.schedule.events[indexes[i]]
                    if (e.ap != -1):
                        holding_time[e.ap] += e.end - e.start
                    
#                    if (d.schedule.events[indexes[i]].start > start_ts):
#                        gothrough[i] = 1
                    indexes[i]+=1
                i+=1
            
        if (max(holding_time) > 0):
            ap = np.argmax(holding_time)
            if ap not in ap_encounter:
                intro.append("\\newmood["+usr.ap[ap]+"]{"+str(len(ap_encounter))+"}{"+color_dico[len(ap_encounter)]+"}\n")
                ap_encounter.append(ap)
            
            
            text.append("\moodday{"+str(datetime.fromtimestamp(start_ts).year)+"-"+str(datetime.fromtimestamp(start_ts).month)+"-"+str(datetime.fromtimestamp(start_ts).day)+"}{"+str(ap_encounter.index(ap))+"}")
            holding_time[ap] = 0
#            if (max(holding_time) > 0):
#                ap = np.argmax(holding_time)
#                if ap not in ap_encounter:
#                    intro.append("\\newmood["+usr.ap[e.ap]+"]{"+str(len(ap_encounter))+"}{"+color_dico[len(ap_encounter)]+"}\n")
#                    ap_encounter.append(ap)
#                text.append("["+str(ap_encounter.index(ap))+"]")
            text.append("\n")
#        already_ready = 0
#        mystr = []
#        j = 0
#        for d in usr.devices:
#            if (d.type == Device.MOBILE):
#                if (gothrough[j] == 1):
#                    if (already_ready == 0):
#                        mystr.append("\moodday{"+str(datetime.fromtimestamp(start_ts).year)+"-"+str(datetime.fromtimestamp(start_ts).month)+"-"+str(datetime.fromtimestamp(start_ts).day)+"}{"+str(j)+"}")
#                        already_ready = 1
#                    elif (already_ready == 1):
#                        mystr.append("["+str(j)+"]")
#                        already_ready = 2
#                j += 1
#        if (already_ready != 0):
#            mystr.append('\n')
#            f.write(''.join(mystr))
            
        start_ts += 86400
        
    f.write(''.join(intro))
    f.write(''.join(text))
        
        
def write_concl(f : IO, usr: User, figs: List[Figure]):
    concl = "\\begin{document}\n\
    \\moodcalendar{2022-01-01}{2022-12-last}\n\
    \\newpage\n\
    \\moodcalendar{2023-01-01}{2023-05-last}\n\
    \\moodlegend\n\
    \\newpage"
    
    for fig in figs:
        concl = concl+fig.get_tex_line()

#    concl = concl+"\\includegraphics[scale=1]{ap_count.png}\n\n\
#    \\includegraphics[scale=1]{daily_ap_histo.png}\n\n\
#    \\includegraphics[scale=1]{devices_ap.png}\n\n\
#    \\includegraphics[scale=0.5]{ap_freq.png}\n\
#    \\includegraphics[scale=0.5]{campus_freq.png}\n\n\
#    \\includegraphics[scale=0.5]{building_freq.png}\n\
#    \\includegraphics[scale=0.5]{floor_freq.png}\n\
    concl = concl+"\\end{document}"
    
    f.write(concl)

def write_latex_file(filename: str, usr: User, figs: List[Figure]) -> None:
    with open(filename, "w") as f:
        f.write(intro)
#        i = 0
#        for d in usr.devices:
#            if (d.type == Device.MOBILE):
#                f.write("\\newmood[Mobile "+str(i)+"]{"+str(i)+"}{"+color_dico[i]+"}\n")
#                i += 1
        
        start = datetime.strptime("2022-01-01", "%Y-%m-%d")
        end = datetime.strptime("2024-01-01", "%Y-%m-%d")
        
        write_schedule(f, usr, start, end)
        
        write_concl(f, usr, figs)
        
#        f.write(concl)

def build_figures(path: str, usr: User) -> List[Figure]:
    i = 0
    
    figures: List[Figure] = []
    for d in usr.devices:
        if (d.type == Device.MOBILE):
#            d.schedule.render_schedule("Schedule of Mobile "+str(i))
#            plt.savefig(path+"schedule_"+str(i)+".png", bbox_inches='tight')
#            figures.append(Figure("schedule_"+str(i)+".png", Figure.FULL))
            
            d.schedule.render_AD_schedule("Arrival/Departure schedule of Mobile "+str(i))
            plt.savefig(path+"ad_schedule_"+str(i)+".png", bbox_inches='tight')
            figures.append(Figure("ad_schedule_"+str(i)+".png", Figure.FULL))
            
            i += 1
        
    usr.render_ap_frequentation()
    plt.savefig(path+"ap_freq.png", bbox_inches='tight')
    figures.append(Figure("ap_freq.png", Figure.HALF))
    
    usr.render_campus_frequentation()
    plt.savefig(path+"campus_freq.png", bbox_inches='tight')
    figures.append(Figure("campus_freq.png", Figure.HALF))
    
    usr.render_building_frequentation()
    plt.savefig(path+"building_freq.png", bbox_inches='tight')
    figures.append(Figure("building_freq.png", Figure.HALF))
    
    usr.render_floor_frequentation()
    plt.savefig(path+"floor_freq.png", bbox_inches='tight')
    figures.append(Figure("floor_freq.png", Figure.HALF))
    
    return figures

def build_latex_file(filename: str, output: str) -> None:
    os.system("pdflatex -output-directory="+output+" -shell-escape -synctex=1 -interaction=nonstopmode "+filename)
