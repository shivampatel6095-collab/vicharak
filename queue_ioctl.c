#ifndef QUEUE_IOCTL_H
#define QUEUE_IOCTL_H

#include <linux/ioctl.h>

#define QM 'a'


struct queue_data {
    int len;
    char __user *data;
};


#define SET_SIZE_OF_QUEUE _IOW(QM, 0x01, int)
#define PUSH_DATA         _IOW(QM, 0x02, struct queue_data)
#define POP_DATA          _IOR(QM, 0x03, struct queue_data)

#endif 
