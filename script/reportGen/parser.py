from usr import *
from typing import Callable, Any, List
import gc

def read_header(lines: List[str]) -> (str, int, int, int):
    l = lines[0].split(':')
    if (l[0] != "UID"):
        raise ValueError("Unable to read the file : file header corrupted")
    
    uid = l[1]
    
    l = lines[1].split(':')
    if (l[0] != "Devices"):
        raise ValueError("Unable to read the file : file header corrupted")
    
    if (not l[1].isdigit()):
        raise TypeError("Unable to read the file : file header corrupted")
    
    device_count = int(l[1])
    
    l = lines[2].split(':')
    if (l[0] != "AP"):
        raise ValueError("Unable to read the file : file header corrupted")
    
    if (not l[1].isdigit()):
        raise TypeError("Unable to read the file : file header corrupted")
        
    ap_count: int = int(l[1])
    
    return uid, device_count, ap_count, 3


def check_marker(marker: str, lines: List[str], index: int) -> (bool, int):
    l = lines[index].split(':')
    index += 1
    return l[0] == marker and l[1] == "begin", index
    
def check_isdigit(s: str, error: str) -> None:
    if (not s.isdigit() and not (s[1:].isdigit() and s[0] == '-')):
        raise TypeError("Unable to read the file : "+error)
        
def read_ap(lines: List[str], index: int) -> (dict, int):
    mark, index = check_marker("AP", lines, index)
    if (not mark):
        raise ValueError("Unable to read the file : AP header corrupted : "+lines[index-1])
    
    ap = dict()
    while (lines[index] != "end"):
        l = lines[index].split(';')
        
        tmp = l[0].split(':')[1]
        name = tmp
        
        tmp = l[1].split(':')[1]
        check_isdigit(tmp, "AP entry corrupted")
        id = int(tmp)
        
        ap[id] = name
        index += 1
    
    index += 1
    return ap, index
        
def read_mac(lines: str, index: int) -> (int, int):
    l = lines[index].split(':')
    index += 1
    if (l[0] != "MAC"):
        raise TypeError("Unable to read the file : device block header corrupted")
    
    return l[1], index
        
def read_device_type(lines: str, index: int) -> (int, int):
    l = lines[index].split(':')
    index += 1
    if (l[0] != "Type"):
        raise TypeError("Unable to read the file : device block header corrupted")
    
    if (l[1] == "MOBILE"):
        return Device.MOBILE, index
    elif (l[1] == "STATIC"):
        return Device.STATIC, index
    elif (l[1] == "UNKNOWN"):
        return Device.UNKNOWN, index
    else:
        raise TypeError("Unable to read the file : device block header corrupted : unknown device type")

def read_schedule(lines: List[str], index: int) -> (Schedule, int):
    mark, index = check_marker("Schedule", lines, index)
    if (not mark):
        raise ValueError("Unable to read the file : schedule block corrupted")
    
    schedule = Schedule()
    while (lines[index] != "end"):
        l = lines[index].split(';')
        
        tmp = l[0].split(':')[1]
        check_isdigit(tmp, "event corrupted")
        start = int(tmp)
        
        tmp = l[1].split(':')[1]
        check_isdigit(tmp, "event corrupted")
        end = int(tmp)
        
        tmp = l[2].split(':')[1]
        if (tmp == "MOVING"):
            type = Event.MOVING
        elif (tmp == "STOP"):
            type = Event.STOP
        else:
            raise ValueError("Unable to read the file : event corrupted")
            
            
        tmp = l[3].split(':')[1]
        check_isdigit(tmp, "event corrupted")
        ap = int(tmp)
        
        schedule.add_event(Event(start, end, type, ap))
        index += 1
    
    index += 1
    return schedule, index
    
def read_ap_freq(lines: List[str], index: int) -> (List[AccessPoint], int):
    mark, index = check_marker("AP", lines, index)
    if (not mark):
        raise ValueError("Unable to read the file : APFreq entry corrupted")
        
    ap_freq: List[AccessPoint] = []
    while (lines[index] != "end"):
        l = lines[index].split(';')
        tmp = l[0].split(':')[1]
        uid = tmp
        
        tmp = l[1].split(':')[1]
        check_isdigit(tmp, "APFreq entry corrupted")
        freq = int(tmp)
        
        ap_freq.append(AccessPoint(uid, freq))
        index += 1
        
    index += 1
    return ap_freq, index

def read_calendar(lines: List[str], index: int, handler: Callable[[List[str]], Any]) -> (Calendar, int):
    calendar = Calendar()
    while (lines[index] != "end"):
        l = lines[index].split(';')
        tmp = l[0].split(':')[1]
        date = datetime.fromisoformat(tmp)
        
        tmp = l[1].split(':')
        val = handler(tmp)
        
        calendar.add_entry(date, val)
        index += 1
    
    index += 1
    return calendar, index
    
    
def ap_count_handler(l: List[str]) -> int:
    if (l[0] != "AP_encountered"):
        raise ValueError("Unable to read the file : APCount entry corrupted")
    
    check_isdigit(l[1], "APCount entry corrupted")
    return int(l[1])
    
def read_ap_count(lines: List[str], index: int) -> (Calendar, int):
    mark, index = check_marker("APCount", lines, index)
    if (not mark):
        raise ValueError("Unable to read the file : APCount header corrupted")
        
    return read_calendar(lines, index, ap_count_handler)
    
    
def ap_swap_handler(l: List[str]) -> int:
    if (l[0] != "AP_swap"):
        raise ValueError("Unable to read the file : APSwap entry corrupted")
    
    check_isdigit(l[1], "APSwap entry corrupted")
    return int(l[1])
    
def read_ap_swap(lines: List[str], index: int) -> (Calendar, int):
    mark, index = check_marker("APSwaps", lines, index)
    if (not mark):
        raise ValueError("Unable to read the file : APSwap header corrupted")
        
    return read_calendar(lines, index, ap_swap_handler)
    
    
def read_device(lines: List[str], index: int) -> (Device, int):
    mark, index = check_marker("Device", lines, index)
    if (not mark):
        raise ValueError("Unable to read the file : device block corrupted")
    
    mac, index = read_mac(lines, index)
    type, index = read_device_type(lines, index)
    schedule, index = read_schedule(lines, index)
#    ap_freq, index = read_ap_freq(lines, index)
    ap_count, index = read_ap_count(lines, index)
    ap_swap, index = read_ap_swap(lines, index)
    
    index += 1
    
    d = Device(mac, type, schedule, None, ap_count, ap_swap)
    
    return d, index
    
    
    
    
def parse(file: str) -> User:
    """ Parse a .usr file """
    usr: User = None
    with open(file, "r") as f:
        data = f.read()
        lines = data.split('\n')
        
        uid, device_count, ap_count, index = read_header(lines)
        aps, index = read_ap(lines, index)
        
        usr = User(uid, ap_count, aps)
        
        for i in range(device_count):
            device, index = read_device(lines, index)
            
            usr.add_device(device)
            
    del lines
    gc.collect()
    return usr
