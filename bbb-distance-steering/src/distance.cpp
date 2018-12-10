#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/spi/spidev.h>
#include <time.h>
#include <sys/time.h>
#include "SimpleGPIO.h"
#include <getopt.h>
#include "SharedMemory.hpp"
#include "CyclicBuffer.hpp"

#define SPI_PATH "/dev/spidev1.0"

#define DEBUG 0

// get distance from distance Arduino, performance/interrupt issues
int compareDistance(unsigned int fd){
       char a[3],startByte;
       // SPI transaction starts with a zero
       read(fd,&startByte,1);
       if(startByte==0x0){
               read(fd,a,3);

                //used for test
                #if (DEBUG)
		    printf("Distance sensors value: %d %d %d\n", a[0], a[1], a[2]);
                #endif

               fflush(stdout);
               return (int)(0 << 24 | a[0] << 16 | a[1] << 8 | a[2]);
       }
       else {
               return -1;// no data get
	}
}

// SS init, all SS pins are ouput and high at the start point
void spiSSInit(int gpio1, int gpio2){
       /*gpio_unexport(gpio1);
       gpio_export(gpio1);
       gpio_unexport(gpio2);
       gpio_export(gpio2);*/

       gpio_set_dir(gpio1, OUTPUT_PIN);
       gpio_set_dir(gpio2, OUTPUT_PIN);

       gpio_set_value(gpio1, HIGH);
       gpio_set_value(gpio2, HIGH);
}



int main(){
       unsigned int fd,i;
       bool canSend = 0;
       uint8_t bits=8, mode=0;
       uint32_t speed=1000000;
       int sentObject,readObject;
       char angle;
       spiSSInit(60, 48);



//---------------- Ask Jacob if this is ok ---------------------------------------------
          CSharedMemory shmMemory_st("/shm_st");
          shmMemory_st.Create(BUFFER_SIZE, O_RDWR);
          shmMemory_st.Attach(PROT_WRITE);
          int* circBufferP_st = (int*)shmMemory_st.GetData();
          Buffer circBuffer_st(BUFFER_SIZE, circBufferP_st, B_CONSUMER);

	  sleep(2);

          CSharedMemory shmMemory_di("/shm_di");
          shmMemory_di.Create(BUFFER_SIZE, O_RDWR);
          shmMemory_di.Attach(PROT_WRITE);
          int* circBufferP_di = (int*)shmMemory_di.GetData();
          Buffer circBuffer_di(BUFFER_SIZE, circBufferP_di, B_PRODUCER);

//-------------------------------------------------------------------------------------

       fd = open(SPI_PATH, O_RDWR);
       // SPI parameter setup
       ioctl(fd, SPI_IOC_WR_MODE, &mode);
       ioctl(fd, SPI_IOC_RD_MODE, &mode);
       ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
       ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
       ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
       ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);


       while (1){
               // get the value from distancce arduino
               gpio_set_value(48, LOW);
               sentObject = compareDistance(fd);
               gpio_set_value(48, HIGH);

              // write distance-packet to shm
              if (sentObject != -1) {
                   shmMemory_di.Lock();
                   circBuffer_di.write(sentObject);
                   shmMemory_di.UnLock();
              }


              // read steering pakcet from shared memory
              shmMemory_st.Lock();
              if(circBuffer_st.getUnreadValues()>0){
                     readObject = circBuffer_st.read();
                     canSend = 1;
              }
              shmMemory_st.UnLock();

              // write steering cmd to arduino_st
              if(canSend){
                     angle = (char)readObject;
                     gpio_set_value(60, LOW);
                     write(fd, &angle, 1);
                     gpio_set_value(60, HIGH);
                     canSend = 0;
              }

              usleep(180000); 
       }
       close(fd);
       return 0;
}
