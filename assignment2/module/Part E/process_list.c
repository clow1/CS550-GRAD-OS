#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/init.h>

#define BUFFER_LENGTH 350

// Declare fop operations
static int pl_open(struct inode *inode, struct file *file);
static ssize_t pl_read(struct file *file, char __user *out, size_t size, loff_t* off);
static int pl_close(struct inode *inodep, struct file *filp);
char* process_state(long state); 

static struct task_struct* p;

static struct file_operations pl_fops = {
	.owner 		= THIS_MODULE,
	.open 		= pl_open,
	.read 		= pl_read,
	.release 	= pl_close
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
	p = next_task(&init_task);
	return 0;
}

static void __exit pl_exit(void) {
	misc_deregister(&pl_device);
	pr_info("Process List Module Unregistered");
}

static int pl_open(struct inode *inode, struct file *file) {
	pr_info("Process List Module Opened\n");
	p = next_task(&init_task);
	return 0;
}

static ssize_t pl_read(struct file *file, char __user *out, size_t size, loff_t* off) {
	int rc;
	int error_count;
	int buffer_size = 0;
    char* p_state = NULL;

    char buffer[BUFFER_LENGTH];

	p_state = process_state(p->state);

	memset(buffer,0,sizeof(char)*BUFFER_LENGTH);
	//PID=1 PPID=0 CPU=4 STATE=TASK_RUNNING
	sprintf(buffer, "PID=%d PPID=%d CPU=%d STATE=%s", p->pid, p->parent->pid, task_cpu(p), p_state);
	//printk(KERN_INFO"\nPID=%d", p->pid);
	buffer_size = strlen(buffer)+1;

	error_count = copy_to_user(out, buffer, buffer_size);
	p = next_task(p);

	if(p == &init_task) {
		rc = 0;
	} else {
		rc = buffer_size;
	}
	
    return rc; 
}

static int pl_close(struct inode *inodep, struct file *filp) {
	pr_info("Process List Module Closed\n");
	p = &init_task;
	return 0;
}

char* process_state(long state) {
	char* rc = "INVALID";
	switch(state) {
		case TASK_RUNNING:
            rc = "TASK_RUNNING";
            break;
        case TASK_INTERRUPTIBLE:
            rc = "TASK_INTERRUPTIBLE";
            break;
        case TASK_UNINTERRUPTIBLE:
            rc = "TASK_UNINTERRUPTIBLE";
            break;
        case __TASK_STOPPED:
            rc = "__TASK_STOPPED";
            break;
        case __TASK_TRACED:
            rc = "__TASK_TRACED";
            break;
		case EXIT_DEAD:
	    	rc = "EXIT_DEAD";
	    	break;
		case EXIT_ZOMBIE:
	    	rc = "EXIT_ZOMBIE";
	    	break;
	    case EXIT_TRACE:
	    	rc = "EXIT_ZOMBIE, EXIT_DEAD";
	    	break;
		case TASK_PARKED:
	    	rc = "TASK_PARKED";
	    	break;
		case TASK_DEAD:
	    	rc = "TASK_DEAD";
	    	break;
		case TASK_WAKEKILL:
	    	rc = "TASK_WAKEKILL";
	    	break;
		case TASK_WAKING:
	    	rc = "TASK_WAKING";
	    	break;
		case TASK_NOLOAD:
	    	rc = "TASK_NOLOAD";
	    	break;
		case TASK_NEW:
	    	rc = "TASK_NEW";
	    	break;
		case TASK_STATE_MAX:
	    	rc = "TASK_STATE_MAX";
	    	break;
	    case TASK_KILLABLE:
	    	rc = "TASK_WAKEKILL, TASK_UNINTERRUPTIBLE";
	    	break;
	    case TASK_STOPPED:
			rc = "TASK_WAKEKILL, __TASK_STOPPED";
			break;
		case TASK_TRACED:
			rc = "TASK_WAKEKILL, __TASK_TRACED";
			break;
		case TASK_IDLE:
			rc = "TASK_UNINTERRUPTIBLE, TASK_NOLOAD"; 
			break;
		case TASK_NORMAL:
			rc = "TASK_INTERRUPTIBLE, TASK_UNINTERRUPTIBLE";
			break;
		case TASK_REPORT:
			rc = "TASK_RUNNING, TASK_INTERRUPTIBLE, TASK_UNINTERRUPTIBLE, __TASK_STOPPED, __TASK_TRACED, EXIT_ZOMBIE, EXIT_DEAD";
			break;
		default:
	    	return "INVALID";
	}
	return rc;
}

module_init(pl_init)
module_exit(pl_exit)
