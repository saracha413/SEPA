# -*- coding: utf-8 -*-
"""
Created on Wed Dec  1 20:39:43 2021

@author: samue

To make this Python script into an executable, ensure that the PyInstaller 
package is installed in your working Python environment and use the following:
    pyinstaller SEPA_TimeLapseTool.py
    
To create the executable, use this command in the Anaconda prompt with the 
appropriate virtual environment activated:
    pyinstaller --noconfirm --onefile --splash SEPA_SplashScreen.png -i D_Pine_CMYK.ico --name SEPA_TimeLapse_Tool SEPA_TimeLapseTool.py
    
See here for more options:
    https://pyinstaller.readthedocs.io/en/stable/usage.html

"""

import numpy as np
import glob
from datetime import datetime
from datetime import timedelta
import re
from PIL import Image
import operator
from tkinter import Tk, filedialog
import sys
import os
import shutil
import cv2
import time
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from PyQt5 import QtGui

# Print fun command line header
print('                                      _.--"""--,    ')
print("                                    .'          `\\ ")
print('  .-""""""-.                      ."              | ')
print(' /          ''.                   /            .-._/')
print('|             `.                |             |     ')
print(' \\             \\          .-._ |          _   \\     Dartmouth Rural STEM Educator Partnership')
print('  """"-.         \_.-.     \   `          ( \__/    Science Education Partnership Award (SEPA)')
print('        |             )     "=.       .,   \        From the Natl. Inst. of General Med. Sciences  ')
print('       /             (         \     /  \  /        Within the National Institutes of Health')
print("     /`               `\        |   /    `'         https://sepa.host.dartmouth.edu/  ")
print("     '..-`\        _.-. `\ _.__/   .=.              2021-2022 Academic Year")
print("          |  _    / \  '.-`    `-.'  /              Created by S. Streeter, December 2021")
print("          \_/ |  |   './ _     _  \.'                 ")
print("               '-'    | /       \ |                   ")
print('                      |  .-. .-.  |                 "IMAGES ARE MORE THAN PICTURES, THEY ARE DATA"')
print('                      \ / o| |o \ /                       --RJ Gillies et al., 2016')
print('                       |   / \   |                    ')
print('                      / """   """ \\                  ')
print("                     /             \\                Welcome to the SEPA Time-Lapse Tool")
print("                    | '._.'         \\                ")
print('                    |  /             |')
print('                     \ |             |')
print('                      ||    _    _   /')
print('                      /|\  (_\  /_) /')
print("                      \ \'._  ` '_.'")
print('                       `""` `"""`        Artwork by Joan G. Stark')
print('||-------------------------------------------------------------------------------------------------||')
print('')

# Close the splash image
try:
    import pyi_splash
    #pyi_splash.update_text('Program loaded!')
    pyi_splash.close()
except:
    pass
        
print('Press ENTER to continue...')
sys.stdout.flush()
junk = input()

# Create interactive window for selecting image files
while True:
    print('Use the separate window to select the folder that contains your time-lapse images (example images')
    print('are found in ./Examples/Example_Image_Data/)...')
    
    root = Tk()     # pointing root to Tk() to use it as Tk() in program.
    root.withdraw() # Hides small tkinter window.
    root.attributes('-topmost', True)         # Opened windows will be active. above all windows despite of selection.
    path_in = '%s/'%filedialog.askdirectory(initialdir=os.getcwd()) # Returns opened path as str
    print('Looking for images in %s...'%path_in)
    
    # Have user confirm image file extension
    ext = input('Type the image file extension (excluding the period symbol, lower/upper case does not matter) and\nhit ENTER: ')    
    
    # Grab zee image files
    Images = glob.glob('%s*.%s'%(path_in,ext))
    N = len(Images)
    if N == 0:
        print('No image files found. Double check selected path and/or image file extension!')
        print('Press ENTER to start again...')
        junk = input()
    else:
        break

print('%i total .%s images found...'%(N,ext))

# Automatically create output subfolder
path_out = '%sSorted_Images/'%(path_in)
isExist = os.path.exists(path_out)
if not isExist:
    os.mkdir(path_out)

# Scrub timestamps from images, copy images, relabel them with timestamps 
ts = np.array([])
for i in range(N):
    year, month, day, hour, minute, second = re.split(' |:', Image.open(Images[i])._getexif()[36867])
    ts = np.append(ts,datetime(int(year),int(month),int(day),int(hour),int(minute),int(second)))
enum_ts = enumerate(ts)
sorted_enum = sorted(enum_ts, key=operator.itemgetter(1))
print('Copying and renaming all images...',end='')
cnt = 0
labeled_images  = []
sorted_idx = []
sorted_ts  = []
for index, element in sorted_enum:
    sorted_idx.append(index)
    sorted_ts.append(element)
    print('%03i%% complete'%round(cnt/N*100),end='')
    fn_in  = Images[index]
    # fn_out = '%s%s.%s'%(path_out,element.strftime("%Y-%m-%d %H.%M.%S"),ext)
    fn_out = '%s%s.%s'%(path_out,round(time.mktime(element.timetuple())),ext)    
    labeled_images.append(fn_out)
    shutil.copy(fn_in, fn_out)
    cnt = cnt + 1
    print('\b\b\b\b\b\b\b\b\b\b\b\b\b',end='')
    sys.stdout.flush()
    
    # DELETE
    # REMOVE ALL IMAGES AFTER CERTAIN IMAGE
    # shutil.copy(fn_in, './Images/A/')
    # if cnt == 350:
    #     sys.exit()
    # # ONLY KEEP ONE IMAGE EVERY FIVE
    # if cnt % 5 == 0:
    #     shutil.copy(fn_in, './Images/A/')
    # # DELETE
print('%03i%% complete'%round(cnt/N*100),end='')
print('')

#------------------------------------------------------------------------------

# Load image data
print('Loading image data...',end='')
I = cv2.imread(labeled_images[0])
height, width, channels = np.shape(I)
data = np.empty([N,height,width,channels],dtype=np.uint8)
cnt = 0
for i in range(N):
    print('%03i%% complete'%round(cnt/N*100),end='')
    data[i] = cv2.imread(labeled_images[i])
    cnt = cnt + 1
    print('\b\b\b\b\b\b\b\b\b\b\b\b\b',end='')
    sys.stdout.flush()
print('%03i%% complete'%round(cnt/N*100),end='')
print('')

#------------------------------------------------------------------------------

# Correct barrel lens distortion 
print('Minimizing lens distortion...',end='')
# REFERENCE THIS: https://stackoverflow.com/questions/26602981/correct-barrel-distortion-in-opencv-manually-without-chessboard-image
# I = data_all[0,:,:,:]
distCoeff = np.zeros((4,1),np.float64)
k1 = -1.0e-7; # negative to remove barrel distortion
k2 = -2.0e-10;
p1 = 0;
p2 = 0;
distCoeff[0,0] = k1;
distCoeff[1,0] = k2;
distCoeff[2,0] = p1;
distCoeff[3,0] = p2;
cam = np.eye(3,dtype=np.float32)
cam[0,2] = width/2.0  # define center x
cam[1,2] = height/2.0 # define center y
cam[0,0] = 10.        # define focal length x
cam[1,1] = 10.        # define focal length y
# dst = cv2.undistort(I,cam,distCoeff)
# plt.figure(1)
# plt.imshow(I)
# plt.figure(2)
# plt.imshow(dst)
cnt = 0
for i in range(N):
    print('%03i%% complete'%round(cnt/N*100),end='')
    data[i,:,:,:] = cv2.undistort(data[i,:,:,:],cam,distCoeff)
    cnt = cnt + 1
    print('\b\b\b\b\b\b\b\b\b\b\b\b\b',end='')
    sys.stdout.flush()
print('%03i%% complete'%round(cnt/N*100),end='')
print('')

#------------------------------------------------------------------------------

# Ask if they would like the images labeled
response = input('Would you like to label each image with a timestamp (y/n)? ') 
while True:
    if response.lower() == 'y' or response.lower() == 'yes':
        break
    elif response.lower() == 'n' or response.lower() == 'no':
        break
    print('Please respond with "y" or "n"...')
    response = input('Would you like to label each image with a timestamp (y/n)? ') 
if response.lower() == 'y' or response.lower() == 'yes':
    
    # Add timestamps and labels to images
    print('Labeling image data...',end='')
    cnt = 0
    for i in range(N):
        print('%03i%% complete'%round(cnt/N*100),end='')
        ts_now = '%s'%sorted_ts[i]
        str_now = 'Frame %i of %i   %s'%(i+1,N,ts_now)
        text_size, _ = cv2.getTextSize(str_now, cv2.FONT_HERSHEY_DUPLEX, 3, 10)
        text_w, text_h = text_size
        cv2.rectangle(data[i], (0,0), (text_w, text_h+10), (1,1,1,0.2), -1)
        cv2.putText(data[i],text=str_now, org = (0,text_h),fontFace=cv2.FONT_HERSHEY_DUPLEX, \
                    fontScale=3,color=(255,255,255),thickness=2,lineType=cv2.LINE_4,bottomLeftOrigin=False);   
        cv2.imwrite(labeled_images[i],data[i])
        cnt = cnt + 1
        print('\b\b\b\b\b\b\b\b\b\b\b\b\b',end='')
        sys.stdout.flush()
    print('%03i%% complete'%round(cnt/N*100),end='')
    print('')

#------------------------------------------------------------------------------

# Provide option to create time-lapse video
response = input('Would you like to create a time-lapse video (y/n)? ') 
while True:
    if response.lower() == 'y' or response.lower() == 'yes':
        break
    elif response.lower() == 'n' or response.lower() == 'no':
        break
    print('Please respond with "y" or "n"...')
    response = input('Would you like to create a time-lapse video (y/n)? ') 
    
if response.lower() == 'y' or response.lower() == 'yes':
    
    # Create separate output subfolder
    path_out2 = '%sTime_Lapse/'%(path_in)
    isExist = os.path.exists(path_out2)
    if not isExist:
        os.mkdir(path_out2)
        
    print('Generating video...',end='')
    fps = 10 # frames per second
    size = (width,height)
    # out = cv2.VideoWriter('%sTime_Lapse.mp4'%path_out,cv2.VideoWriter_fourcc(*'DIVX'), fps, size)
    # out = cv2.VideoWriter('%sTime_Lapse.mp4'%path_out2,cv2.VideoWriter_fourcc(*'XVID'), fps, size)
    out = cv2.VideoWriter('%sTime_Lapse.mp4'%path_out2,cv2.VideoWriter_fourcc(*'mp4v'), fps, size)
    cnt = 0
    for i in range(N):
        print('%03i%% complete'%round(cnt/N*100),end='')
        out.write(data[i])
        cnt = cnt + 1
        print('\b\b\b\b\b\b\b\b\b\b\b\b\b',end='')
        sys.stdout.flush()
    out.release()
    print('%03i%% complete'%round(cnt/N*100),end='')
    print('')

#------------------------------------------------------------------------------

# Free up 
#------------------------------------------------------------------------------
print('||-------------------------------------------------------------------------------------------------||')
print('')
print('Time to setup ImageJ. Hit ENTER after each step is complete to continue...')
print('')
print('   1.  Start ImageJ by double-clicking on ImageJ.exe (found in ImageJ subfolder).')
print('')
junk = input()
print('   2.  In the ImageJ interface, select File-->Import-->Image Sequence...')
print('')
junk = input()
print('   3.  Browse to the folder containing your images and select the Sorted_Images subfolder.')
print('')
junk = input()
print('   4.  Hit OK. It may take a moment to load all the images (note the progress bar along the bottom')
print('       of ImageJ interface window).')
print('')
junk = input()
print('||-------------------------------------------------------------------------------------------------||')
print('')
print('The first step is to determine the spatial scale of the time-lapse images. To do this, we will')
print('need to measure the 1cm horizontal and/or vertical grid lines in the background.')
print('')
print('   1.  Select the STRAIGHT LINE drawing tool from the ImageJ menu bar.')
print('')
junk = input()
print('   2.  Click and drag along the length of one square in the background grid pattern.')
print('')
junk = input()
print('   3.  Release the mouse button and hit CTRL+m. A Results table should appear containing the')
print('       measurement.')
print('')
junk = input()
print('   4.  Repeat steps 2 and 3 four or five times measuring vertical and horizontal grid squares in')
print('       different places.')
print('')
junk = input()
print('   5.  In the Results table window, in the menu bar, select Results-->Summarize.')
print('')
junk = input()
print('   6.  In the Results table, in the row titled "Mean," take note of the LAST value in the rightmost')
print('       colummn titled "Length". (You will need this number in the next step.)')
print('')
junk = input()
print('   7.  In the main ImageJ interface menu bar, select Analyze-->Set Scale...')
print('')
junk = input()
print('   8.  Input the mean length from step 6 in the "Distance in pixels" box.')
print('')
junk = input()
print('   9.  Input the number "1" in the "Known distance" box.')
print('')
junk = input()
print('   10. Input "cm" in the "Unit of length" box.')
print('')
junk = input()
print('   11. Make sure that the "Global" box is checked.')
print('')
junk = input()
print('   12. Hit OK in the Set Scale window. Now the images are in units of cm!')
print('')

#------------------------------------------------------------------------------
print('||-------------------------------------------------------------------------------------------------||')
print('')
print('Time to perform ImageJ measurements. Hit ENTER after each step is complete to continue...')
print('')
print('   1.  In the Results table window menu bar, select Results-->Clear Results. Choose to not save the')
print('       results.')
print('')
junk = input()
print('   2.  In the main ImageJ interface window, click and hold the STRAIGHT LINE drawing tool button.')
print('       Select the SEGMENTED LINE option from the dropdown.')
print('')
junk = input()
print('   3.  Using the slider bar at the bottom of the ImageJ image stack window, inspect the time-lapse')
print('       images. Focus on a SINGLE seedling/plant. What we need to do is measure the length of your')
print('       target seedling/plant at different time points.')
print('')
junk = input()
print('   4.  When you wish to take a measurement, starting at the base of a plant, repeatedly LEFT click')
print('       to trace the length of the plant. The last tracing click must be a RIGHT click. This ends')
print('       the tracing. Hit CTRL+m to record the measurement.')
print('')
junk = input()
print('   5.  Left click once anywhere in the image window to clear the previous selection.')
print('')
junk = input()
print('   6.  Repeat steps 4 and 5 for several images at different points in time.')
print('')
junk = input()
print('   7.  When you are done, in the Results table window, select File-->Save As. Adjust the file name')
print('       so that it notes your name and/or the seedling you measured. For example, "Results_Sam.csv"')
print('       or "Results_Sam_Seedling1.csv". The file will be saved in the Sorted_Images subfolder')
print('')
junk = input()

#------------------------------------------------------------------------------
print('||-------------------------------------------------------------------------------------------------||')
print('')
print('Select the saved results CSV file to plot the measurements...')
root = Tk()     # pointing root to Tk() to use it as Tk() in program.
root.withdraw() # Hides small tkinter window.
root.attributes('-topmost', True)         # Opened windows will be active. above all windows despite of selection.
file_in = '%s'%filedialog.askopenfilename(initialdir=os.getcwd()) # Returns filename
print('Loading tabulated measurements...')
data = pd.read_csv(file_in)
print(data)

# Sort the data such that the measurements are in chronological order - doing 
# this enables one to take measurements in an order desired
data = data.sort_values('Label',ascending=True)

# response = input('Would you like to label daylight hours in your plots (y/n)? ') 
# while True:
#     if response.lower() == 'y' or response.lower() == 'yes':
#         break
#     elif response.lower() == 'n' or response.lower() == 'no':
#         break
#     print('Please respond with "y" or "n"...')
#     response = input('Would you like to label daylight hours in your plots (y/n)? ') 
response = 'n'

print('Generating plots...')
sys.stdout.flush()

# Convert timestamps of measurements
dates = []
for i in range(len(data)):
    dates.append(datetime.fromtimestamp(int(data['Label'].values.tolist()[i].replace('Sorted_Images:',''))))

# Isolate metrics of interest
plant_Length           = data['Length'].to_numpy()
plant_HeightWidthRatio = data['Height'].to_numpy() / data['Width'].to_numpy()

# Figure font sizes
FSZ1 = 15
FSZ2 = 10

#------------------------------------------------------------------------------

# Make plot - date and time of each ImageJ measurement taken

# Round timestamps to nearest hour
time_photo = []
for i in range(len(dates)):
    timestamp_now = dates[i]
    if timestamp_now.minute >= 30 and timestamp_now.hour < 23:
        timestamp_now = timestamp_now.replace(second=0, microsecond=0, minute=0, hour=timestamp_now.hour+1)
    elif timestamp_now.minute >= 30 and timestamp_now.hour == 23:
        timestamp_now = timestamp_now + timedelta(days=1)
        timestamp_now = timestamp_now.replace(second=0, microsecond=0, minute=0, hour=0)
    else:
        timestamp_now = timestamp_now.replace(second=0, microsecond=0, minute=0)
    time_photo.append(timestamp_now)
    
# Create hour-by-hour datetime array from start of imaging to end of imaging    
time_start = dates[0].replace(microsecond=0, second=0, minute=0, hour=0)
time_stop  = (dates[-1] + timedelta(days=1)).replace(microsecond=0, second=0, minute=0, hour=0)
time_array = [time_start]
time_now   = time_start
while time_now < time_stop:
    time_now += timedelta(hours=1)
    time_array.append(time_now)
   
# Create "daytime" array for optional visualization
daytime_array = np.zeros(len(time_array))
for i in range(len(time_array)):
    if time_array[i].hour >= 8 and time_array[i].hour <= 16:
        daytime_array[i] = 1

# Create binary array to indicate when photos were taken
photo_array = np.zeros(len(time_array))
cnt = 0
i = 0
while cnt < len(time_photo):
    if time_array[i] == time_photo[cnt] and cnt < len(time_photo):
        # print('i=%i, cnt=%i'%(i,cnt))
        photo_array[i] = 1
        cnt = cnt + 1
    i = i + 1

# Make plot - when photos were taken
fig, ax = plt.subplots()
plt.get_current_fig_manager().window.setWindowIcon(QtGui.QIcon('D_Pine_CMYK.png'))
plt.get_current_fig_manager().set_window_title('SEPA Time-Lapse Tool: When ImageJ measurements were taken')
for i in range(len(time_array)):
    if photo_array[i] == 1:
        plt.scatter(time_array[i],photo_array[i],s=50,marker='.',color=[1, 0, 0, 1],zorder=100,edgecolors='k',label='_nolegend_')
    else:
        plt.scatter(time_array[i],photo_array[i],s=50,marker='.',color=[0, 0, 0, 1],zorder=100,edgecolors='k',label='_nolegend_')

# Add daylight labeling if desired
flag = 0
if response.lower() == 'y' or response.lower() == 'yes':
    for i in range(len(time_array)):
        if daytime_array[i] == 1:
            if flag == 0:
                plt.plot([time_array[i], time_array[i]],[-10, 10],color=[1, 1, 0, 0.2],linewidth=3,zorder=5,label='Daylight')
                flag = 1
            else:
                plt.plot([time_array[i], time_array[i]],[-10, 10],color=[1, 1, 0, 0.2],linewidth=3,zorder=5,label='_nolegend_')

# Other plotting stuff
ax.xaxis.set_major_locator(mdates.DayLocator())
ax.xaxis.set_minor_locator(mdates.HourLocator(byhour=(6, 12, 18)))
ax.grid(True,zorder=2)
plt.xlabel('Timestamp',fontsize=FSZ1)
plt.ylabel("Measurement Taken?",fontsize=FSZ1)
ax.xaxis.set_major_formatter(mdates.DateFormatter('%b %d %Y'))
plt.xticks(fontsize=FSZ2,rotation=30,horizontalalignment='right')
plt.ylim([-0.5,1.5])
ax.set_yticks([0,1])
ax.set_yticklabels(['No','Yes'])
plt.yticks(fontsize=FSZ1)
plt.tight_layout()
if response.lower() == 'y' or response.lower() == 'yes':
    h, l = ax.get_legend_handles_labels()
    plt.legend(fontsize=FSZ1,facecolor=[1,1,1,0],framealpha=1)
plt.draw()
plt.savefig("%sMeasurement_Taken.tif"%path_out, dpi=200, facecolor='w', edgecolor='w',bbox_inches='tight')

#------------------------------------------------------------------------------

# Make plot - length
fig, ax = plt.subplots()
plt.get_current_fig_manager().window.setWindowIcon(QtGui.QIcon('D_Pine_CMYK.png'))
plt.get_current_fig_manager().set_window_title('SEPA Time-Lapse Tool: Plant length and growth rate')
plt.scatter(dates,plant_Length,s=50,marker='.',color=[0, 0, 0, 1],zorder=100,label='_nolegend_')
plt.plot(dates,plant_Length,color=[0, 0, 0, 1],zorder=90,linestyle=':',linewidth=1,label='_nolegend_')

# Add daylight labeling if desired
flag = 0
if response.lower() == 'y' or response.lower() == 'yes':
    for i in range(len(time_array)):
        if daytime_array[i] == 1:
            if flag == 0:
                plt.plot([time_array[i], time_array[i]],[-100, 100],color=[1, 1, 0, 0.2],linewidth=3,zorder=5,label='Daylight')
                flag = 1
            else:
                plt.plot([time_array[i], time_array[i]],[-100, 100],color=[1, 1, 0, 0.2],linewidth=3,zorder=5,label='_nolegend_')

# Other plotting stuff
ax.xaxis.set_major_locator(mdates.DayLocator())
ax.xaxis.set_minor_locator(mdates.HourLocator(byhour=(6, 12, 18)))
ax.grid(True)
plt.xlabel('Timestamp',fontsize=FSZ1)
plt.ylabel("Plant Length (cm)",fontsize=FSZ1)
ax.xaxis.set_major_formatter(mdates.DateFormatter('%b %d %Y'))
plt.xticks(fontsize=FSZ2,rotation=30,horizontalalignment='right')
plt.yticks(fontsize=FSZ1)
plt.ylim([0, np.max(plant_Length)+1])
#plt.tight_layout()
if response.lower() == 'y' or response.lower() == 'yes':
    h, l = ax.get_legend_handles_labels()
    plt.legend(fontsize=FSZ1,facecolor=[1,1,1,0],framealpha=1)
plt.draw()
plt.savefig("%sPlant_Length.tif"%path_out, dpi=200, facecolor='w', edgecolor='w',bbox_inches='tight')

#------------------------------------------------------------------------------

# Add to previous plot - growth rate
dl = np.diff(plant_Length)
dl = np.insert(dl, 0, np.nan)
dt = np.diff(dates)
dates_seconds = np.zeros(len(dt))
for i  in range(len(dt)):
    dates_seconds[i] = dt[i].total_seconds()
dt = np.insert(dates_seconds, 0, np.nan)
rate = dl/dt * 60 * 60 * 10 # mm/hour growth rate
ax2 = ax.twinx()
plt.scatter(dates,rate,s=50,marker='.',color=[0, 0, 1, 1],zorder=100,label='_nolegend_')
plt.plot(dates,rate,color=[0, 0, 1, 1],zorder=90,linestyle=':',linewidth=1,label='_nolegend_')
ax2.set_ylabel('Growth Rate (mm/hr)',fontsize=FSZ1,color='b')
plt.yticks(fontsize=FSZ1,color='b')
plt.tight_layout()
plt.draw()
plt.savefig("%sPlant_Length_GrowthRate.tif"%path_out, dpi=200, facecolor='w', edgecolor='w',bbox_inches='tight')

#------------------------------------------------------------------------------

# Make plot - height to width ratio
fig, ax = plt.subplots()
plt.get_current_fig_manager().window.setWindowIcon(QtGui.QIcon('D_Pine_CMYK.png'))
plt.get_current_fig_manager().set_window_title('SEPA Time-Lapse Tool: Plant height-to-width ratio')
plt.scatter(dates,plant_HeightWidthRatio,s=50,marker='.',color=[0, 0, 0, 1],zorder=100,label='_nolegend_')
plt.plot(dates,plant_HeightWidthRatio,color=[0, 0, 0, 1],zorder=90,linestyle=':',linewidth=1,label='_nolegend_')

# Add daylight labeling if desired
flag = 0
if response.lower() == 'y' or response.lower() == 'yes':
    for i in range(len(time_array)):
        if daytime_array[i] == 1:
            if flag == 0:
                plt.plot([time_array[i], time_array[i]],[-100, 100],color=[1, 1, 0, 0.2],linewidth=3,zorder=5,label='Daylight')
                flag = 1
            else:
                plt.plot([time_array[i], time_array[i]],[-100, 100],color=[1, 1, 0, 0.2],linewidth=3,zorder=5,label='_nolegend_')

# Other plotting stuff
ax.xaxis.set_major_locator(mdates.DayLocator())
ax.xaxis.set_minor_locator(mdates.HourLocator(byhour=(6, 12, 18)))
ax.grid(True)
plt.xlabel('Timestamp',fontsize=FSZ1)
plt.ylabel("Plant H/W Ratio (unitless)",fontsize=FSZ1)
ax.xaxis.set_major_formatter(mdates.DateFormatter('%b %d %Y'))
plt.xticks(fontsize=FSZ2,rotation=30,horizontalalignment='right')
plt.yticks(fontsize=FSZ1)
plt.ylim([0, np.max(plant_HeightWidthRatio)+1])
plt.tight_layout()
if response.lower() == 'y' or response.lower() == 'yes':
    h, l = ax.get_legend_handles_labels()
    plt.legend(fontsize=FSZ1,facecolor=[1,1,1,0],framealpha=1)
plt.draw()
plt.savefig("%sPlant_HWratio.tif"%path_out, dpi=200, facecolor='w', edgecolor='w',bbox_inches='tight')

print('Close all figure windows when done inspecting measurements...')
sys.stdout.flush()
plt.show()

#------------------------------------------------------------------------------

print('Done!')
print('')
print('||-------------------------------------------------------------------------------------------------||')
print('')
print("      _.-'''''-._")
print("    .'  _     _  '.")
print("   /   (o)   (o)   \\  CONGRATULATIONS! You have completed the SEPA Time-Lapse Tool exercise.")
print("  |                 | Hit ENTER to end the program.")
print("  |  \           /  |")
print("   \  '.       .'  /")
print("    '.  `'---'`  .'")
print("jgs   '-._____.-'")
sys.stdout.flush()
junk = input()

#------------------------------------------------------------------------------