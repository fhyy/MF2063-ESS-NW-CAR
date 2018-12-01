SOURCES = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)

OBJECTS = $(SOURCES:%.cpp=%.o)
PROGRAM = motor_speed_spi

CC := g++
CFLAGS =
LIBS = pthread rt
LDFLAGS = $(LIBS:%=-l%)

$(PROGRAM) : $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)
    
%.o : %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

run : $(PROGRAM)
	./motor_speed_spi
    
.PHONY : clean
clean :
	rm -f $(PROGRAM) $(OBJECTS)

