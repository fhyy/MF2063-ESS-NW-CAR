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

#define SPI_PATH "/dev/spidev1.0"

int main(){
        unsigned int fd,i;
        char  a[3],b;
        uint8_t bits=8, mode=0;
        uint32_t speed=1000000;
        int distance;
        uint8_t rx[2]={0},tx[2]={0};
        double begin,end;
        struct timeval tv;

        struct spi_ioc_transfer tr;
        fd = open(SPI_PATH, O_RDWR);
        ioctl(fd, SPI_IOC_WR_MODE, &mode);
        ioctl(fd, SPI_IOC_RD_MODE, &mode);
        ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);

         for(i=0;i<20;i++){
                read(fd,&b,1);
                if(b==0x0){
                        read(fd,&a,3);
                        printf("%d %d %d\n",a[0],a[1], a[2]);
                        fflush(stdout);
                        usleep(100000);
                }else
                        break;
        }
        close(fd);
        return 0;
}

