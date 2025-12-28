#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h> 
#include<string.h>
#include "queue_ioctl.h"
#define DRIVER_NAME "/dev/vicharak"
int main(){
    int fd = open(DRIVER_NAME,0_RDWR);
    struct data d;
    char *buf=malloc(3);
    memcpy(buf,"xyz",3);
    d.len=3;
    d.data=buf;
    ioctl(fd,PUSH_DATA,&d);
    close(fd);
    free(buf);
    return 0;
    
}