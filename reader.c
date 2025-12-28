#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h> 
#include "queue_ioctl.h"
#define DRIVER_NAME "/dev/vicharak"
int main(){
    int fd = open(DRIVER_NAME,0_RDWR);
    struct data d;
    char *buf=malloc(10);
   
    d.len=10;
    d.data=buf;
    ioctl(fd,POP_DATA,&d);
    printf("popped:%.*s\n",10,buf);
    close(fd);
    free(buf);
    return 0;
    
}