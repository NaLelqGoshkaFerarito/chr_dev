#include "chr_dev.h"
#include <linux/init.h>
#include <linux/module.h>

MODULE_DESCRIPTION("Final assignment driver");
//GNU public license, used in order to avoid warnings
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vladislav Serafimov");

static int cd_driver_init(void){
    printk(KERN_NOTICE "Serafimov-driver: Initializing...");
    return register_device();
}

static void cd_driver_exit(void){
    printk(KERN_NOTICE "Serafimov-driver: Exiting...");
    unregister_device();
}

module_init(cd_driver_init);
module_exit(cd_driver_exit);