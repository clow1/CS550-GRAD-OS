#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>

char buffer[1000];

static int my_open(struct inode *inode, struct file *file)
{
	pr_info("Morning, pal!\n");
	return 0;
}

static int my_close(struct inode *inodep, struct file *filp)
{
	pr_info("Good night....\n");
	return 0;
}
static ssize_t my_read(struct file *file, char __user* out, size_t size, loff_t* off)
{
	int error_ct = 0;
	int len = 0;
	sprint(buffer, "Hello World!\n");
	len = strlen(buffer) + 1;
	error_count = copy_to_user(out, buffer, len);
	return len;

}


static ssize_t my_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	pr_info("Took in %d bytes\n", len);
	return len;
}

static const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.write = my_write,
	.open = my_open,
	.release = my_close,
	.llseak = no_llseek,
};

struct miscdevice my_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "my_simple_misc",
	.fops = &my_fops,
};

static int __init misc_init(void)
{
	int error;
	error = misc_register(&my_device);
	if (error) {
		pr_err("misc_register failed.\n");
		return error;
	}
	pr_info("Successfully registered\n");
	return 0;
}

static void __exit misc_exit(void)
{
	misc_deregister(&my_device);
	pr_info("Later!\n");
}

module_init(misc_init)
module_exit(misc_exit)

MODULE_DESCRIPTION("Simple Misc Driver");
MODULE_AUTHOR("Crystal Low <clow1@binghamton.edu>");
MODULE_LICENSE("GPL");
