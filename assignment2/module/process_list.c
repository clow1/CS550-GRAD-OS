#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

static struct miscdevice pl = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "process_list",
	.fops = &pl_fops
};
static struct file_operations pl_fops = {
	.owner = THIS_MODULE,
	.open = pl_open,
	.release = pl_close,
	.read = pl_read,
	.llseek = noop_llseek

};

char* buffer[1000];
static int __init pl_init() 
{
	misc_register(&pl);
}

static void __exit pl_exit(void)
{
	misc_deregister(&pl);
}

static int pl_open(struct inode *inode, struct file *file) 
{
	pr_info("process list character device opened\n");



	return 0;
}
static int pl_close(struct inode *inodep, struct file *filp) {
	pr_info("process list module closed\n");
	return 0;
}



static ssize_t pl_read(struct file *file, char __user *out, size_t size, loff_t* off)
{
	sprint(buffer, "xxxxxxoxoxoxo\n");
	if (access_OK()) 
	{

		copy_to_user(out,buffer,strlen(buffer)+1);
	}

}

