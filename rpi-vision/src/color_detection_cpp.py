"""@package docstring
Documentation for this module.

Blablabla
"""

# python color_tracking.py

# import the necessary packages
import numpy as np
import imutils
import cv2
import time
import sys

## @var lower
# define the lower boundaries of the colors in the HSV color space
lower = {'1': (166, 84, 141), '2': (66, 122, 129), '3': (23, 59, 119)}

## @var upper
# define the upper boundaries of the colors in the HSV color space
upper = {'1': (186, 255, 255), '2': (86, 255, 255),  '3': (54, 255, 255)}

## @var colors
# define standard colors for circle around the object
colors = {'1': (0, 0, 255), '2': (0, 255, 0),  '3': (0, 255, 217)}

## @var camera
# Object representing the camera
camera = cv2.VideoCapture(0)

streamBlock = False
streamCount = 0

# keep looping
while True:
    nothingDetected = True

    # grab the current frame
    (grabbed, frame) = camera.read()
    frame = imutils.resize(frame, width=600)
    blurred = cv2.GaussianBlur(frame, (11, 11), 0)
    hsv = cv2.cvtColor(blurred, cv2.COLOR_BGR2HSV)
    # for each color in dictionary check object in frame
    for key, value in upper.items():

        # construct a mask for the color from dictionary`1, then perform
        # a series of dilations and erosions to remove any small
        # blobs left in the mask
        kernel = np.ones((9, 9), np.uint8)
        mask = cv2.inRange(hsv, lower[key], upper[key])
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel)
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel)

        # find contours in the mask and initialize the current
        # (x, y) center of the ball
        cnts = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL,
                                cv2.CHAIN_APPROX_SIMPLE)[-2]
        center = None

        # only proceed if at least one contour was found
        if len(cnts) > 0:
            # find the largest contour in the mask, then use
            # it to compute the minimum enclosing circle andq
            # centroid
            c = max(cnts, key=cv2.contourArea)
            ((x, y), radius) = cv2.minEnclosingCircle(c)

            M = cv2.moments(c)
            (a, b) = (int(M["m10"] / M["m00"]), int(M["m01"] / M["m00"]))
            if a < 200:
                loc = '1'
            elif a > 400:
                loc = '2'
            else: # object is in the middle
                loc = '3'
            # only proceed if the radius meets a minimum size. Correct this value for your obect's size
            if radius > 0.5:
                # draw the circle and centroid on the frame,
                # then update the list of tracked points
                if streamCount < 5:
                    print(key + loc)
                    sys.stdout.flush()
                    streamBlock = False
                    nothingDetected = False
                    streamCount += 1
                else:
                    print('44\n')
                    sys.stdout.flush()
                    streamBlock = False
                    nothingDetected = False
                    streamCount = 0
    if (nothingDetected == True) and (streamBlock == False):
        print('44' + '\n' + '\n')
        #print("No signal detected!")
        sys.stdout.flush()
        streamBlock = True

    time.sleep(0.5)
