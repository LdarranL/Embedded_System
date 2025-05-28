#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "hello_drv"
static int major_num;
static char device_buffer[4] = {0};

static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "hello_drv: Device opened\n");
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, 
                       size_t len, loff_t *offset) {
    if (copy_to_user(buffer, device_buffer, 4)) {
        return -EFAULT;
    }
    printk(KERN_INFO "hello_drv: Read %d bytes\n", 4);
    return 4;
}

static ssize_t dev_write(struct file *filep, const char *buffer,
                        size_t len, loff_t *offset) {
    if (copy_from_user(device_buffer, buffer, 4)) {
        return -EFAULT;
    }
    printk(KERN_INFO "hello_drv: Wrote %d bytes\n", 4);
    return 4;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "hello_drv: Device closed\n");
    return 0;
}

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int __init hello_init(void) {
    major_num = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_num < 0) {
        printk(KERN_ALERT "hello_drv: Registration failed\n");
        return major_num;
    }
    printk(KERN_INFO "hello_drv: Registered with major %d\n", major_num);
    return 0;
}

static void __exit hello_exit(void) {
    unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_INFO "hello_drv: Unregistered\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");

