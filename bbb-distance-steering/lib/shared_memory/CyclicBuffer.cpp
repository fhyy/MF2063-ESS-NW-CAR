#include "CyclicBuffer.hpp"

#define DEBUG 0

//Constructor that takes a size and a pointer to existing allocated memory for the buffer
//Notice that the process doing the reading needs to call this constructor earlier in time than the process doing the writing or it brakes functionality
Buffer::Buffer(unsigned size, int* circBuffP, int permission) : memorySize(size), bufferSize(size - OVERHEAD_SIZE)
{
    //If we read, we dont have access to write in this segment (i.e. segmentation fault)
    //The first tree values of the buffer tells us:
    //[0]; the amount of unread values in the buffer
    //[1]; the read index of the buffer (tail)
    //[2]; the write index of the buffer (head)
    //[3..n] values written to the buffer
    if(DEBUG)
        printf("Size of buffer is: %d\n", bufferSize);
    buffer = circBuffP;
    if(permission == B_PRODUCER){
        buffer[UNREAD_INDEX] = 0;
        buffer[READ_INDEX] = READ_START;
        buffer[WRITE_INDEX] = WRITE_START;
    }
    if(DEBUG)
        printf("buffer value:%d \n", *buffer);
    //Set unique pointers to the buffer-status values
    unreadValues = &buffer[UNREAD_INDEX];
    if(DEBUG)
        printf("unreadValues value:%d\n", *unreadValues);
    readIndex = &buffer[READ_INDEX];
    writeIndex = &buffer[WRITE_INDEX];
}

//Write a single int to the buffer
void Buffer::write(int input)
{
    //Check if we are overwriting values that are yet to be read and should thus move the read pointer as well as it always should read the "first in" value.
    if(*unreadValues == bufferSize) {
        //Wrap around to beginning of buffer if we are at final location of memory (same as final location of buffer)
        if(++(*readIndex) == memorySize){
            *readIndex = READ_START;
        }
    } else {
        //We didnt overwrite an existing value, so there is an additional value that needs reading
        (*unreadValues)++;
    }
    if(DEBUG)
        printf("Writing %d at position %d\n", input, *writeIndex);
    buffer[(*writeIndex)++] = input;
    //Check if we have written to the last place in the buffer and should start from the beginning
    if(*writeIndex == memorySize){
        *writeIndex = WRITE_START;
    }
}

//Read a single int from the buffer, checking if values are available needs to be done before calling read using Buffer:getUnreadValues()
int Buffer::read()
{
    int val = buffer[(*readIndex)++];
    if(DEBUG)
        printf("Read %d at position %d with %d more values to read  \n", val, (*readIndex) - 1, *unreadValues);
    //Check if we have read from the last place in the buffer and should start from the beginning
    if(*readIndex == memorySize)
        *readIndex = READ_START;

    (*unreadValues)--;
    return val;
}

//See how many unread values are stored in the buffer
int Buffer::getUnreadValues()
{
    return *unreadValues;
}

