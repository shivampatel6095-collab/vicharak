#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include "queue_ioctl.h"
#define DRIVER_NAME "/dev/vicharak"
int main(){
    int fd = open(DRIVER_NAME,0_RDWR);
    int size = 100;
    ioctl(fd,SET_SIZE_OF_QUEUE,&size);
    close(fd);
    return 0;
    
}