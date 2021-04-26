#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>

char buffer[1000];

// Declare fop operations
static int pl_open(struct inode *inode, struct file *file);
static int pl_close(struct inode *inodep, struct file *filp);
static ssize_t pl_read(struct file *file, char __user *out, size_t size, loff_t* off);


static struct file_operations pl_fops = {
	.owner 		= THIS_MODULE,
	.open 		= pl_open,
	.release 	= pl_close,
	.read 		= pl_read,
	.llseek 	= no_llseek
};

static struct miscdevice pl_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "process_list",
	.fops = &pl_fops
};

static int __init pl_init(void) {
	int error;
	error = misc_register(&pl_device);

	if(error) {
		pr_err("FAILED: Register Process List Module Unsuccessful");
		return error;
	}

	pr_info("SUCCESS: Process List Module Registered");
	return 0;
}

static void __exit pl_exit(void) {
	misc_deregister(&pl_device);
	pr_info("Process List Module Unregistered");
}

static int pl_open(struct inode *inode, struct file *file) {
	pr_info("Process List Module Opened\n");
	return 0;
}

static int pl_close(struct inode *inodep, struct file *filp) {
	pr_info("Process List Module Closed\n");
	return 0;
}

//PID=1 PPID=0 CPU=4 STATE=TASK_RUNNING

static ssize_t pl_read(struct file *file, char __user *out, size_t size, loff_t* off) {
	int error_count = 0;
    int buffer_size = 0;
    struct task_struct* p;

    for_each_process(p) {
    	sprintf(buffer, "PID=%d", p->pid);
    }

    sprintf(buffer, "Hello World");
    
    buffer_size = strlen(buffer)+1;
    error_count = copy_to_user(out, &buffer, buffer_size);
    return buffer_size; 
}

module_init(pl_init)
module_exit(pl_exit)