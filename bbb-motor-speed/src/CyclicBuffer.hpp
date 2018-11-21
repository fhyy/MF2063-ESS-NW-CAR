#ifndef __CIRCULARBUFFER_H
#define __CIRCULARBUFFER_H

#include <stdlib.h>
#include <cstdio>

#define BUFFER_START 3
#define READ_START 3
#define WRITE_START 3
#define READ_INDEX 1
#define WRITE_INDEX 2
#define UNREAD_INDEX 0
#define B_PRODUCER 1
#define B_CONSUMER 0
#define OVERHEAD_SIZE 3 

//Class that implements a circular buffer
class Buffer
{
public:
    Buffer(unsigned int size, int* circBuffP, int permission);
    void write(int input);
    int read();
    int getUnreadValues();
private:
    //The first tree values of the buffer tells us:
    //[0]; the amount of unread values in the buffer
    //[1]; the read index of the buffer
    //[2]; the write index of the buffer
    //[3..n] values written to the buffer
    int* buffer;
    int* unreadValues;
    int* readIndex;
    int* writeIndex;
    unsigned int size;
    //Total size of allocated memory (buffer + overhead for readindex, writeindex, unreadvalues)
    unsigned int memorySize;
    //Size of the buffer
    unsigned int bufferSize;
};
 
#endif //__CIRCULARBUFFER_H
