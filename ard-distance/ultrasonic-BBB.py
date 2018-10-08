# ultrasonic-BBB.py : BBB side code for reading data 
#                       from arduino mircro connected with 
#                       ultrasonnic sensors
# 
# Authors:
#        Yifan Ruan (yifanr@kth.se)
#
# license:
#

import serial
import Adafruit_BBIO.UART as UART
from time import sleep

UART.setup("UART1")
ser=serial.Serial(port='/dev/ttyO1', baudrate=9600)
ser.close()

ser.open()
# current code aims to verify the communication between BBB and Arduino boards,
#       var count will be removed in real application
count = 100
f=open('/usr/games/distance.txt','w')
while(count):
        while ser.inWaiting()==0:
                pass
        #dataline = ser.read()
        dataline = ser.readline()
        print dataline
        f.write(dataline)
        count = count - 1
ser.close()
f.close()


