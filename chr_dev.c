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

//struct for writing a character to an index
struct int_ch{
    int val_int;
    char val_ch;
};

//unique magic number
#define M_NUM 238
#define R_CHAR _IOR(M_NUM, 1, int *)
#define W_CHAR _IOW(M_NUM, 2, struct int_ch *)


static long int df_ioctl(struct file* fp, unsigned cmd, unsigned long arg){
    //user space variables stored in here
    struct int_ch temp;
    switch(cmd){
        case R_CHAR:
            //get value of argument
            if (copy_from_user(&temp.val_int, (int *)arg, sizeof(temp.val_int)) != 0) return -EFAULT;
            //read character at index if in range
            if (temp.val_int < g_s_welcome_sz)
                printk(KERN_NOTICE "Serafimov-driver: Character at [%d] is [%c]\n", temp.val_int, g_s_welcome[temp.val_int]);
            else printk(KERN_NOTICE "Serafimov-driver: Out of bounds read, returning\n");
            return 0;
            break;
        case W_CHAR:
            //copy char and int from user
            if (copy_from_user(&temp, (struct int_ch *)arg, sizeof(temp)) != 0) return -EFAULT;
            //write if in range
            if (temp.val_int < g_s_welcome_sz) {
                printk(KERN_NOTICE
                "Serafimov-driver: Writing [%c] at [%d]\n", temp.val_ch, temp.val_int);
                g_s_welcome[temp.val_int] = temp.val_ch;
            }
            else printk(KERN_NOTICE "Serafimov-driver: Out of bounds write, returning\n");
            return 0;
            break;
        default:
            printk(KERN_NOTICE "Serafimov-driver: Non-existent ioctl call\n");
            return -ENOTTY;
    }
}

static struct file_operations serafimov_driver_fops =
        {
        .owner = THIS_MODULE,
        .read = df_read,
        .write = df_write,
        .open = df_open,
        .release = df_release,
        .unlocked_ioctl = df_ioctl,
        };

static const char name[] = "serafimov_driver";
static const char name_class[] = "serafimov_class";

#define N_OF_DEVICES 2
static struct cdev ser_dev[N_OF_DEVICES];
static struct class *ser_class;
static dev_t nums;

int register_device(void){
    dev_t curr_dev;
    //register device
    alloc_chrdev_region(&nums, 0, N_OF_DEVICES, name);
    //register device class
    ser_class = class_create(THIS_MODULE, name_class);
    if (ser_class == NULL || MAJOR(nums) < 0) return -1;
    for (int i = 0; i < N_OF_DEVICES; ++i) {
        //initialize device with the given file ops
        cdev_init(&ser_dev[i], &serafimov_driver_fops);
        curr_dev = MKDEV(MAJOR(nums), MINOR(nums) + i);
        //create devices with uniqe ids
        device_create(ser_class, NULL, , NULL, "serafimov_dev%d", i);
        printk(KERN_NOTICE "Serafimov-driver: Registered device MAJ:[%d], MIN:[%d]\n", MAJOR(curr_dev), MINOR(curr_dev));
        //make device accessible
        cdev_add(&ser_dev[i], MKDEV(MAJOR(nums), MINOR(curr_dev) + i), 1);
    }
    return 0;
}


void unregister_device(void){
    printk(KERN_NOTICE "Serafimov-driver: Unregistering device\n");
    if(MAJOR(nums) != 0){
        unregister_chrdev_region(0, N_OF_DEVICES);
        for (int i = 0; i < N_OF_DEVICES; ++i) {
            cdev_del(&ser_dev[i]);
        }
        class_destroy(ser_class);
    }
}