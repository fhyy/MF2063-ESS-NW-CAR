#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/spi/spidev.h>
#include <time.h>
#include <sys/time.h>
#include "SimpleGPIO.h"
#include "SharedMemory.hpp"
#include "CyclicBuffer.hpp"

#define SPI_PATH "/dev/spidev1.0"
#define EVER (;;)
#define MOTOR_PIN 60
#define SENSOR_PIN 48

#define DEBUG 0

//Initialize a GPIO-pin to act as a Slave Select (SS)
void spiSSInit(int gpio)
{
        /*gpio_unexport(gpio);
        gpio_export(gpio);*/
        gpio_set_dir(gpio, OUTPUT_PIN);
        gpio_set_value(gpio, HIGH);
}

// SPI parameter setup, takes a file descriptor to the SPI interface
void spiInit(unsigned int fd)
{
        //GPIO-Pin 60 (P9-12) is slave select for the motor controller
        //GPIO-Pin 48 (P9-15) is slave select for the speed sensor
        //Initiates all slave-select pins so we can set them manually to the chip we want to communicate with
        spiSSInit(MOTOR_PIN);   //pin 60
        spiSSInit(SENSOR_PIN);  //Pin 48

        //SPI parameters
        uint8_t bits=8, mode=0;
        uint32_t speed=1000000;
        //Spi parameter setup
        ioctl(fd, SPI_IOC_WR_MODE, &mode);
        ioctl(fd, SPI_IOC_RD_MODE, &mode);
        ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
}

//To start spi communication we set the correct SlaveSelect gpio-pin to low
void spiStart(int gpio)
{
        gpio_set_value(gpio, LOW);
}

//To stop spi communication we set the correct SlaveSelect gpio-pin to high
void spiStop(int gpio)
{
        gpio_set_value(gpio, HIGH);
}

//We send a given value to the motor controller arduino over spi
void sendMotorValue(unsigned int fd, unsigned char controlValue)
{
        spiStart(MOTOR_PIN);
        write(fd, &controlValue, 1);
        spiStop(MOTOR_PIN);
}

//We read a single speed value from the given arduino over spi
int readSensorValue(unsigned int fd)
{
        int received = 0;
        // Talk to the speed sensor by enabling its slave select (active low)
        spiStart(SENSOR_PIN);
        //Read received value pointed to by the file descriptor
        read(fd, &received, 1);
        //stop SPI transfer
        spiStop(SENSOR_PIN);

        return received;
}

//This program intermediately reads a speed value from the speed sensor and sends a desired control variable to the motor controller. The desired control variable can be in terms of distance or speed depending on the implementation of the car controller which is implemented on another beaglebone. The communication between the arduinos and this beaglebone is done over SPI.
int main(void)
{
        unsigned int fd,i;
        char receivedMessage, targetSpeed, currentSpeed;
        bool canSend = 0;

        fd = open(SPI_PATH, O_RDWR);
        // SPI parameter setup
        spiInit(fd);

        // shared memory initialization, hint: first CONSUMER then PRODUCER
        CSharedMemory shmMemory_mo("/shm_mo");
        shmMemory_mo.Create(BUFFER_SIZE, O_RDWR);
        shmMemory_mo.Attach(PROT_WRITE);
        int* circBufferP_mo = (int*)shmMemory_mo.GetData();
        Buffer circBuffer_mo(BUFFER_SIZE, circBufferP_mo, B_CONSUMER);
        sleep(2);
        CSharedMemory shmMemory_sp("/shm_sp");
        shmMemory_sp.Create(BUFFER_SIZE, O_RDWR);
        shmMemory_sp.Attach(PROT_WRITE);
        int* circBufferP_sp = (int*)shmMemory_sp.GetData();
        Buffer circBuffer_sp(BUFFER_SIZE, circBufferP_sp, B_PRODUCER);


        //Endless loop
        for EVER {

                //Read the sensor value given by the spedometer
                receivedMessage = readSensorValue(fd);
                // the 7th bit of receivedMessage should be 1
                currentSpeed = receivedMessage >> 1;
                if(currentSpeed > 125){
                        currentSpeed = 125;
                }
                if(DEBUG){
		    printf("######## Half of Sensor value was: %d\n", receivedMessage);
                }
                // update current speed in each iteration
                currentSpeed = (1<<7) | currentSpeed;
                sendMotorValue(fd, currentSpeed);
                //write to the named pipe to send the information over vsomeip
                shmMemory_sp.Lock();
                circBuffer_sp.write((int) receivedMessage);
                shmMemory_sp.UnLock();

                shmMemory_mo.Lock();
                if(circBuffer_mo.getUnreadValues()>0){
                int tmp = circBuffer_mo.read();
                     targetSpeed = (char)tmp;
                     canSend = 1;
                }
                shmMemory_mo.UnLock();

                //Send the values to the arduino motor controller
                if(canSend){
                        targetSpeed = targetSpeed >> 1;
                        sendMotorValue(fd, targetSpeed);
                        canSend = 0;
                }
                //We sleep so we dont interrupt the arduino to often and it never gets to do work
                usleep(200000);
        }

        return 1;
}
