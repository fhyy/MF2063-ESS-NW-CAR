#include "SharedMemory.hpp"
#include "CyclicBuffer.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#define OVERHEAD_SIZE 3 
#define BUFFER_SIZE 4 + OVERHEAD_SIZE


using namespace std;


int main(int argc, char* argv[])
{

    //Create circular buffer
    try {

        //Write data to shared mem if CLI argument1 is a 1
        if(std::string(argv[1]) =="1") {
            sleep(2);

            //init shared memory
            CSharedMemory shmMemory("/shm_di");
            shmMemory.Create(BUFFER_SIZE, O_RDWR);
            shmMemory.Attach(PROT_WRITE);
            int* circBufferP = (int*)shmMemory.GetData();
            Buffer circBuffer(BUFFER_SIZE, circBufferP, B_PRODUCER);

            //Test shared mem
            for(;;) {
                char sTemp[10];
                //If the program preivously have crashed this semaphore is not unlocked and the program hangs. To fix, remove file /shm/sem.semaphoreInit and run again
                shmMemory.Lock();
                int temp = rand()%100;
                //printf("Writing: %d\n", temp);
                circBuffer.write(temp);
                shmMemory.UnLock();
                sleep(2);
            }


        } else {
            //init shared memory
            CSharedMemory shmMemory("/shm_st");
            shmMemory.Create(BUFFER_SIZE, O_RDWR);
            shmMemory.Attach(PROT_WRITE);
            int* circBufferP = (int*)shmMemory.GetData();
            Buffer circBuffer(BUFFER_SIZE, circBufferP, B_CONSUMER);

            for(;;) {
                char sTemp[10];
                shmMemory.Lock();
                int values = circBuffer.getUnreadValues();
                if (values > 0) {
                    //printf("Reading");
                    int value = circBuffer.read();
                    //printf("---> %d\n",value);
                }
                shmMemory.UnLock();
                sleep(4);
            }
        }

    }
    catch (std::exception& ex) {
        cout<<"Exception:"<<ex.what();
    }

}
