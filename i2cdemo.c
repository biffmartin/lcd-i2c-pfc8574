#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <sys/ioctl.h>
#include "linux/i2c-dev.h"

/* ------------------------------------------------
 * Demonstrate general IO to the LCD
 * ----------------------------------------------- */

int main(int argc, char *argv[])
{
    int fd;
    struct timespec _500ms;
    _500ms.tv_sec = 0;
    _500ms.tv_nsec = 5000000L;
    fd = open("/dev/i2c-1", O_RDWR);
    if(fd < 0) {
        fprintf(stderr, "Error opening device\n");
        exit(EXIT_FAILURE);
    }
    if(ioctl(fd, I2C_SLAVE, 0x20) < 0) {
        fprintf(stderr, "Error setting slave address\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    char buf[10];
    buf[0] = 0x07; // IODIR PORT1
    buf[1] = 0x00; // all output
    if(write(fd, buf, 2) != 2) {
        fprintf(stderr, "Error writing (1)\n");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Write success!\n");

    nanosleep(&_500ms, NULL);

    buf[0] = 0x01; // PORT1
    buf[1] = 0xff; // output high
    if(write(fd, buf, 2) != 2) {
        fprintf(stderr, "Error writing (2)\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    printf("Done.\n");
    exit(EXIT_SUCCESS);
}
