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

#define SPI_PATH "/dev/spidev1.0"

// get distance from distance Arduino, performance/interrupt issues
char* compareDistance(unsigned int fd){
        char a[3],startByte;
        // SPI transaction starts with a zero
        read(fd,&startByte,1);
        if(startByte==0x0){
                read(fd,&a,3);
                fflush(stdout);
                return a;
        }
        else
                return -1;// no data get
        /*if (a[1] < 5 ){
                return 50;// obstacle in front of the car, d
        }else if ((a[1]-a[0]) > 20){
                return 100;// obstacle on the left, turn right
        }else if ((a[1]-a[2]) > 20){
                 return 0;// obstacle on the right, turn left
        }else
                return 50;
        */

}

// SS init, all SS pins are ouput and high at the start point 
void spiSSInit(int gpio1, int gpio2){
        gpio_export(gpio1);
        gpio_export(gpio2);

        gpio_set_dir(gpio1, OUTPUT_PIN);
        gpio_set_dir(gpio2, OUTPUT_PIN);

        gpio_set_value(gpio1, HIGH);
        gpio_set_value(gpio2, HIGH);
}



int main(){
        unsigned int fd,i;

        uint8_t bits=8, mode=0;
        uint32_t speed=1000000;
        char* result;
        int sentObject;
        spiSSInit(60, 48);

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
                result = compareDistance(fd);
//              printf("%d\n",result);
                gpio_set_value(48, HIGH);
                sentObject = (char)1 << 24 | a[0] << 16 | a[1] << 8 | a[3];
                
                // some other works need to be done.




                usleep(500000); // half seconds

        }
        close(fd);
        return 0;
}



