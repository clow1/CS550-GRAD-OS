#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
MODULE_LICENSE("Dual BSD/GPL");

#define BUFFER_LENGTH 256

// Declare fop operations
static int pl_open(struct inode *inode, struct file *file);
static int pl_close(struct inode *inodep, struct file *filp);
static ssize_t pl_read(struct file *file, char *out, size_t size, loff_t* off);
static char* process_state(long state);


static struct task_struct* proc;

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
	proc = next_task(&init_task); //we set process proc to point to the process after the first process.
	
	return 0;
}

static void __exit pl_exit(void) {
	misc_deregister(&pl_device);
	pr_info("Process List Module Unregistered");
}

static int pl_open(struct inode *inode, struct file *file) {
	pr_info("Process List Module Opened\n");
	proc = next_task(&init_task);
	return 0;
}

static int pl_close(struct inode *inodep, struct file *filp) {
	pr_info("Process List Module Closed\n");
	return 0;
}

//PID=1 PPID=0 CPU=4 STATE=TASK_RUNNING

static ssize_t pl_read(struct file *file, char *out, size_t size, loff_t* off) {
    int error_count = 0;
    int buffer_size = 0;
   
//    char* p_state=NULL;

    char buffer[BUFFER_LENGTH];
    struct task_struct* p;

    //memset(buffer,0,sizeof(char)*BUFFER_LENGTH);
    for_each_process(p) {

    	if (proc == p) {	//for verification starting from the first task, the init_task. In other words, we KNOW that proc at FIRST references to 
				// the task that is after that of &init_task, and then consequently, we may traverse along each process p more properly.
		char* p_state =process_state(p->state);
		memset(buffer, 0, sizeof(char) * BUFFER_LENGTH);
		sprintf(buffer, "PID=%d  PPID=%d CPU = %d STATE = %s\n", p->pid, p->parent->pid, task_cpu(p),p_state);
		error_count = copy_to_user(out, buffer, strlen(buffer)+1);
		if (error_count != 0) {
			printk(KERN_ALERT "Cannot copy data to user program");
			return EFAULT; //copy_to_user error, probably an invalid reference
		}
		proc=next_task(p); //proc is now being defined as the next process relative to p
		break;
	}
  }
  return strlen(buffer);	
}

char* process_state(long state) {
	char* rv;
	switch(state) {
		case TASK_RUNNING:
            rv = "TASK_RUNNING";
           return rv;
        case TASK_INTERRUPTIBLE:
            rv = "TASK_INTERRUPTIBLE";
            return rv;
        case TASK_UNINTERRUPTIBLE:
            rv = "TASK_UNINTERRUPTIBLE";
            return rv;
        case __TASK_STOPPED:
            rv = "__TASK_STOPPED";
            return rv;
        case __TASK_TRACED:
            rv = "__TASK_TRACED";
            return rv;
		case EXIT_DEAD:
	    	rv = "EXIT_DEAD";
	    	return rv;
		case EXIT_ZOMBIE:
	    	rv = "EXIT_ZOMBIE";
	    	return rv;
	    case EXIT_TRACE:
	    	rv = "EXIT_ZOMBIE, EXIT_DEAD";
	    	return rv;
		case TASK_PARKED:
	    	rv = "TASK_PARKED";
	    	return rv;
		case TASK_DEAD:
	    	rv = "TASK_DEAD";
	    	return rv;
		case TASK_WAKEKILL:
	    	rv = "TASK_WAKEKILL";
	    	return rv;
		case TASK_WAKING:
	    	rv = "TASK_WAKING";
	    	return rv;
		case TASK_NOLOAD:
	    	rv = "TASK_NOLOAD";
	    	return rv;
		case TASK_NEW:
	    	rv = "TASK_NEW";
	    	return rv;
		case TASK_STATE_MAX:
	    	rv = "TASK_STATE_MAX";
	    	return rv;
	    case TASK_KILLABLE:
	    	rv = "TASK_WAKEKILL, TASK_UNINTERRUPTIBLE";
	    	return rv;
	    case TASK_STOPPED:
			rv = "TASK_WAKEKILL , __TASK_STOPPED";
			return rv;
		case TASK_TRACED:
			rv = "TASK_WAKEKILL , __TASK_TRACED";
			return rv;
		case TASK_IDLE:
			rv = "TASK_UNINTERRUPTIBLE , TASK_NOLOAD"; 
			return rv;
		case TASK_NORMAL:
			rv = "TASK_INTERRUPTIBLE , TASK_UNINTERRUPTIBLE";
			return rv;
		case TASK_REPORT:
			rv = "TASK_RUNNING , TASK_INTERRUPTIBLE , TASK_UNINTERRUPTIBLE , __TASK_STOPPED , __TASK_TRACED , EXIT_ZOMBIE , EXIT_DEAD";
			return rv;
		default:
	    	return "INVALID";
	}
}

module_init(pl_init)
module_exit(pl_exit)
