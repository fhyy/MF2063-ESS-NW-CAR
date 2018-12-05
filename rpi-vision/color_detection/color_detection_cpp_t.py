# python color_tracking.py

# import the necessary packages
import numpy as np
import imutils
import cv2
from subprocess import Popen, PIPE

print("INFO: Initialize color space")
# define the lower and upper boundaries of the colors in the HSV color space
lower = {'1': (166, 84, 141), '2': (66, 122, 129), '3': (23, 59, 119)}
upper = {'1': (186, 255, 255), '2': (86, 255, 255),  '3': (54, 255, 255)}

# define standard colors for circle around the object
colors = {'1': (0, 0, 255), '2': (0, 255, 0),  '3': (0, 255, 217)}
print("INFO: starting camera")
camera = cv2.VideoCapture(0)
p = Popen(['./camera-service'], stdout=PIPE, stdin=PIPE)

# keep looping
while True:
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
            if a < 300:
                loc = '1'
            if a > 300:
                loc = '2'
            if a == 300:
                loc = '3'
            # only proceed if the radius meets a minimum size. Correct this value for your obect's size
            if radius > 0.5:
                # draw the circle and centroid on the frame,
                # then update the list of tracked points
                output = key + loc + '\n'
            else:
                output = '\n'
            output = bytes(output, 'UTF-8')
            p.stdin.write(output)
            p.stdin.flush()
            # ONLY FOR TEST(DELETE WHEN USING)
            result = p.stdout.readline().strip()
            print(result)