#include "chr_dev.h"
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>


static const char g_s_welcome[] = "Henlo, this is driver >.<\n\0";
static const ssize_t g_s_welcome_sz = sizeof(g_s_welcome);

static ssize_t df_read(struct file *fp, char __user *buffer, size_t count, loff_t *pos){
    printk(KERN_NOTICE "Serafimov-driver: Read at offset [%i]. Read bytes [%u]", (int)(*pos), (unsigned int) count);
    //if done reading
    if (*pos >= g_s_welcome_sz) return 0;
    //if trying to read out of bounds read only as much as available
    if (*pos + count > g_s_welcome_sz) count = g_s_welcome_sz - *pos;
    //if the read fails return bad address error
    if (copy_to_user(buffer, g_s_welcome + *pos, count) != 0) return -EFAULT;
    *pos += count;
    return count;
}


static struct file_operations serafimov_driver_fops =
        {
        .owner = THIS_MODULE,
        .read = df_read,
        };

static int maj_num = 0;
static const char name[] = "Serafimov-driver";


int register_device(void){
    //get major number
    int ret_val = register_chrdev(0, name, &serafimov_driver_fops);
    printk(KERN_NOTICE "Serafimov-driver: Registering device\n");
    //major number < 0 means error
    if (ret_val < 0){
        printk(KERN_WARNING "Serafimov-driver: Cannot register device (Error code [%i], Line [%i])\n", ret_val, __LINE__);
        return ret_val;
    }
    maj_num = ret_val;
    printk(KERN_NOTICE "Serafimov-driver: Registered device (Major number [%i]\n", maj_num);
    return 0;
}


void unregister_device(void){
    printk(KERN_NOTICE "Serafimov-driver: Unregistering device\n");
    if(maj_num != 0){
        unregister_chrdev(maj_num, name);
    }
}