This software is the tool for my investigation of the Eduroam network.

# Usage

## Main code

### Preparation

To use this software properly, you have to fill the constants defined in 'main.c' as follow :

- INPUT_AP_DATABASE : path to the csv file holding AP details
- INPUT_DEVICE_DATABASE : path to the actual database
- USER_LOG_OUTPUT_DIR : path to the output directory (where the .usr file will be stored)

If you want to use the render engine to visualize the dataset, please do as follow :
- download the glfw and the vulkan SDKs
- remove the comments of lines 62-65 of the CMakeLists.txt file
- remove the comments of the last two lines of the src/CMakeLists.txt file
- remove the comments of the 13 and 43 lines of the src/main.c file

### Compilation

To compile this code, just produce a new directory and use cmake :
```
$ mkdir build && cd build
$ cmake .. && make
```

## Scripts

### General remarks

I've written several script ranging from sanitazing script to rendering scripts,
feal free to take a look around but most of the code is messy

### Sanitizer

- fix_database : update teh second column of the database with the correct timestamp (some of them were corrupted for some reasons)
- filter : a small tool to extract a small amount of data from the database for demonstration purposes
- logfreq : preliminary work to identify the inter and intra AP logs distribution
- discretize : produce a discretize version of the database (15mins buckets)
- change_mac_addr_format : reformat the MAC address of the AP database to be compatible with the device database

### ReportGen

A small report generator which produce a pdf file per user. The main file is resume_generator and uses the output of the main code to produce those stats.
Use it as follow :
```
$ python3 resume_generator.py user_id.usr outputdir
```

Please make sure to fill the right path in tex.py for the moodtracker.sty file ! (and also that you have latex intalled on you computer)

### Distance

A small tool to produce the matrix image of the AP graph. VERY UNSTABLE AND NOT PORTABLE.

### AP distribution

Produce graph for AP count and AP swap of devices in the database

### Schedule

Ugly code, don't go there ...

### Build reports

Automatic bash scripts to generate all the reports with ReportGen

# Remarks

The software produce a backup file to avoid recomputing between two runs.
This file is located in the same directory as the database and is a
hidden file with the same name with .graph as a suffix.
