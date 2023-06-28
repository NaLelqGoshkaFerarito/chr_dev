#include "chr_dev.h"
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>


static char g_s_welcome[] = "Henlo, this is driver >.<\n\0";
static const ssize_t g_s_welcome_sz = sizeof(g_s_welcome);

static int df_open(struct inode *inodest, struct file *fp){
    printk(KERN_NOTICE "Serafimov-driver: Opened [%s]\n", fp->f_path.dentry->d_iname);
    return 0;
}

static int df_release(struct inode *inodest, struct file *fp){
    printk(KERN_NOTICE "Serafimov-driver: Releasing [%s]\n", fp->f_path.dentry->d_iname);
    return 0;
}

static ssize_t df_read(struct file *fp, char __user *buffer, size_t count, loff_t *pos){
    printk(KERN_NOTICE "Serafimov-driver: Read at offset [%i]. Read bytes [%u]\n", (int)(*pos), (unsigned int) count);
    //if done reading
    if (*pos >= g_s_welcome_sz) return 0;
    //if trying to read out of bounds read only as much as available
    if (*pos + count > g_s_welcome_sz) count = g_s_welcome_sz - *pos;
    //if the read fails return bad address error
    if (copy_to_user(buffer, g_s_welcome + *pos, count) != 0) return -EFAULT;
    *pos += count;
    return count;
}

static ssize_t df_write(struct file *fp, const char __user *buffer, size_t count, loff_t *pos){
    printk(KERN_NOTICE "Serafimov-driver: Write at offset [%i]. Wrote [%u] bytes\n", (int)(*pos), (unsigned int) count);
    ssize_t len = min(g_s_welcome_sz - *pos, count);
    if (len <= 0) return 0;
    if (copy_from_user(g_s_welcome + *pos, buffer, len) != 0) return -EFAULT;
    printk(KERN_NOTICE "Serafimov-driver: Finished writing to user\n");
    *pos += len;
    return len;
}

static struct file_operations serafimov_driver_fops =
        {
        .owner = THIS_MODULE,
        .read = df_read,
        .write = df_write,
        .open = df_open,
        .release = df_release,
        };

static const char name[] = "Serafimov-driver";
static const char name_class[] = "Serafimov-class";

static struct cdev ser_dev;
static struct class *ser_class;
static dev_t nums;

int register_device(void){
    //register device
    alloc_chrdev_region(&nums, 0, 1, name);
    //register device class
    ser_class = class_create(THIS_MODULE, name_class);
    if (ser_class == NULL || MAJOR(nums) < 0) return -1;
    //initialize device with the given file ops
    cdev_init(&ser_dev, &serafimov_driver_fops);
    device_create(ser_class, NULL, nums, NULL, "Serafimov-device");
    printk(KERN_NOTICE "Serafimov-driver: Registered device\n");
    cdev_add(&ser_dev, nums, 1);
    return 0;
}


void unregister_device(void){
    printk(KERN_NOTICE "Serafimov-driver: Unregistering device\n");
    if(MAJOR(nums) != 0){
        unregister_chrdev_region(0, 1);
        cdev_del(&ser_dev);
        class_destroy(ser_class);
    }
}