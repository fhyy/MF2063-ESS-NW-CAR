#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/spi/spidev.h>

#define SPI_PATH "/dev/spidev1.0"

int main(){
        unsigned int fd,i;
        char a;
        uint8_t bits=8, mode=0;
        uint32_t speed=10000;
        int distance;
        uint8_t rx[4],tx[4];
        struct spi_ioc_transfer tr;
        fd = open(SPI_PATH, O_RDWR);
        ioctl(fd, SPI_IOC_WR_MODE, &mode);
        ioctl(fd, SPI_IOC_RD_MODE, &mode);
        ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);

        tr.tx_buf = (unsigned long)tx;
        tr.rx_buf = (unsigned long)rx;
        tr.len = 1;
        tr.speed_hz = 10000000;
        tr.bits_per_word = 8;
        tr.delay_usecs = 0;
        tr.rx_buf = (unsigned long)rx;
        tr.len = 1;
        tr.speed_hz = 10000000;
        tr.bits_per_word = 8;
        tr.delay_usecs = 0;
        for(i=0;i<20;i++){
                read(fd,&a,1);
                printf("%d\n\n",a);
                fflush(stdout);
                usleep(100000);
        }
        close(fd);
        return 0;
}

    