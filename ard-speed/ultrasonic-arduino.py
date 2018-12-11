import serial
import Adafruit_BBIO.UART as UART
from time import sleep

UART.setup("UART1")
ser=serial.Serial(port='/dev/ttyO1', baudrate=9600)
ser.close()

ser.open()
while(1):
        while ser.inWaiting()==0:
                pass
        #dataline = ser.read()
        dataline = ser.readline()
        print dataline
