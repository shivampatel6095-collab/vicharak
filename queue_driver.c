#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/slab.h>
#include <linux/mutex.h>

#include "queue_ioctl.h"

#define DEVICE_NAME "vicharak"

/* Device globals */
static dev_t dev_num;
static struct class *qclass;
static struct cdev qcdev;
static struct device *qdevice;

/* Queue structures */
struct queue_node {
    char *buf;
    int len;
};

struct queue_ctrl {
    struct queue_node *nodes;
    int size;
    int head;
    int tail;
    int count;
    int initialized;
    struct mutex lock;
    wait_queue_head_t wq;
};

static struct queue_ctrl q;

/* File operations */
static int q_open(struct inode *i, struct file *f)
{
    return 0;
}

static int q_close(struct inode *i, struct file *f)
{
    return 0;
}

/* IOCTL handler */
static long q_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    struct queue_data d;
    int qsize;
    char *kbuf;

    switch (cmd) {

    case SET_SIZE_OF_QUEUE:
        if (copy_from_user(&qsize, (int __user *)arg, sizeof(int)))
            return -EFAULT;

        if (qsize < 1)
            return -EINVAL;

        mutex_lock(&q.lock);

        /* Free existing queue */
        if (q.initialized) {
            int i;
            for (i = 0; i < q.count; i++) {
                kfree(q.nodes[(q.head + i) % q.size].buf);
            }
            kfree(q.nodes);
        }

        q.nodes = kmalloc_array(qsize, sizeof(struct queue_node), GFP_KERNEL);
        if (!q.nodes) {
            mutex_unlock(&q.lock);
            return -ENOMEM;
        }

        memset(q.nodes, 0, qsize * sizeof(struct queue_node));
        q.size = qsize;
        q.head = q.tail = q.count = 0;
        q.initialized = 1;

        mutex_unlock(&q.lock);
        return 0;

    case PUSH_DATA:
        if (!q.initialized)
            return -EINVAL;

        if (copy_from_user(&d, (void __user *)arg, sizeof(d)))
            return -EFAULT;

        if (d.len <= 0)
            return -EINVAL;

        kbuf = kmalloc(d.len, GFP_KERNEL);
        if (!kbuf)
            return -ENOMEM;

        if (copy_from_user(kbuf, d.data, d.len)) {
            kfree(kbuf);
            return -EFAULT;
        }

        mutex_lock(&q.lock);

        if (q.count == q.size) {
            mutex_unlock(&q.lock);
            kfree(kbuf);
            return -ENOSPC;
        }

        q.nodes[q.tail].buf = kbuf;
        q.nodes[q.tail].len = d.len;
        q.tail = (q.tail + 1) % q.size;
        q.count++;

        mutex_unlock(&q.lock);
        wake_up_interruptible(&q.wq);
        return 0;

    case POP_DATA:
        if (!q.initialized)
            return -EINVAL;

        if (copy_from_user(&d, (void __user *)arg, sizeof(d)))
            return -EFAULT;

        /* Block until data available */
        wait_event_interruptible(q.wq, q.count > 0);

        mutex_lock(&q.lock);

        kbuf = q.nodes[q.head].buf;
        d.len = q.nodes[q.head].len;

        q.nodes[q.head].buf = NULL;
        q.head = (q.head + 1) % q.size;
        q.count--;

        mutex_unlock(&q.lock);

        if (copy_to_user(d.data, kbuf, d.len)) {
            kfree(kbuf);
            return -EFAULT;
        }

        if (copy_to_user((void __user *)arg, &d, sizeof(d))) {
            kfree(kbuf);
            return -EFAULT;
        }

        kfree(kbuf);
        return 0;
    }

    return -EINVAL;
}

/* File operations table */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = q_open,
    .release = q_close,
    .unlocked_ioctl = q_ioctl,
};

/* Module init */
static int __init q_init(void)
{
    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);

    cdev_init(&qcdev, &fops);
    cdev_add(&qcdev, dev_num, 1);

    qclass = class_create(THIS_MODULE, DEVICE_NAME);
    qdevice = device_create(qclass, NULL, dev_num, NULL, DEVICE_NAME);

    mutex_init(&q.lock);
    init_waitqueue_head(&q.wq);

    q.initialized = 0;
    q.nodes = NULL;

    return 0;
}

/* Module exit */
static void __exit q_exit(void)
{
    int i;

    if (q.nodes) {
        for (i = 0; i < q.count; i++)
            kfree(q.nodes[(q.head + i) % q.size].buf);
        kfree(q.nodes);
    }

    device_destroy(qclass, dev_num);
    class_destroy(qclass);
    cdev_del(&qcdev);
    unregister_chrdev_region(dev_num, 1);
}

module_init(q_init);
module_exit(q_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vicharak_user");
MODULE_DESCRIPTION("Dynamic Blocking Circular Queue Char Device");
