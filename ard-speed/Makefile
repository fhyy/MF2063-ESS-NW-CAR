INO = ard-speed.ino
CC = arduino
BOARD = arduino:avr:micro
PORT := $(shell ls /dev/tty* | grep "ttyACM")

upload:
	$(CC) -v --board $(BOARD) --port $(PORT) --upload $(INO)

verify: $(INO)
	$(CC) -v --board $(BOARD) --verify $< 

output: upload
	cat $(PORT)

plot: upload
	cat $(PORT) | feedgnuplot --lines --points --legend 0 "data 0" --title "Test plot" --y2 1 --unset grid --terminal 'dumb 80,40' --exit --stream 0.3

communication: upload
	screen $(PORT) 115200

