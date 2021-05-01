#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>

#define BUFFER_LENGTH 1000

// Declare fop operations
static int pl_open(struct inode *inode, struct file *file);
static int pl_close(struct inode *inodep, struct file *filp);
static ssize_t pl_read(struct file *file, char __user *out, size_t size, loff_t* off);
char* process_state(long state); 

static struct task_struct* p;

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
	p = next_task(&init_task);
	pr_info("SUCCESS: Process List Module Registered");
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

static int pl_close(struct inode *inodep, struct file *filp) {
	pr_info("Process List Module Closed\n");
	p = &init_task;
	return 0;
}

//PID=1 PPID=0 CPU=4 STATE=TASK_RUNNING

static ssize_t pl_read(struct file *file, char __user *out, size_t size, loff_t* off) {
	int error_count;
    //char* state = NULL;

    char buffer[BUFFER_LENGTH];
    struct task_struct* p2;
    //struct task_struct* p_parent;

    int buffer_size = strlen(buffer);

    for_each_process(p) {
    	if(p == p2){
    		// Get parrent PID
	    	//p_parent = p->parent;
	    	// Get process state
	    	//state = process_state(p->state);
    		memset(buffer,0,sizeof(char)*BUFFER_LENGTH);
	    	sprintf(buffer, "PID=%d", p->pid);
	    	error_count = raw_copy_to_user(out, buffer, buffer_size);
	    	if (error_count != 0) {
	    		pr_err("FAILED: Failed to write data to end user");
	    		return errno;
	    	}

	    	p = next_task(p2);
	    	break;
    	}

    }
    return buffer_size; 
}

char* process_state(long state) {
	char* rv;
	switch(state) {
		case TASK_RUNNING:
            rv = "TASK_RUNNING";
            break;
        case TASK_INTERRUPTIBLE:
            rv = "TASK_INTERRUPTIBLE";
            break;
        case TASK_UNINTERRUPTIBLE:
            rv = "TASK_UNINTERRUPTIBLE";
            break;
        case __TASK_STOPPED:
            rv = "__TASK_STOPPED";
            break;
        case __TASK_TRACED:
            rv = "__TASK_TRACED";
            break;
		case EXIT_DEAD:
	    	rv = "EXIT_DEAD";
	    	break;
		case EXIT_ZOMBIE:
	    	rv = "EXIT_ZOMBIE";
	    	break;
	    //case EXIT_TRACE:
	    	//rv = "EXIT_ZOMBIE, EXIT_DEAD";
	    	//break;
		case TASK_PARKED:
	    	rv = "TASK_PARKED";
	    	break;
		case TASK_DEAD:
	    	rv = "TASK_DEAD";
	    	break;
		case TASK_WAKEKILL:
	    	rv = "TASK_WAKEKILL";
	    	break;
		case TASK_WAKING:
	    	rv = "TASK_WAKING";
	    	break;
		case TASK_NOLOAD:
	    	rv = "TASK_NOLOAD";
	    	break;
		case TASK_NEW:
	    	rv = "TASK_NEW";
	    	break;
		case TASK_STATE_MAX:
	    	rv = "TASK_STATE_MAX";
	    	break;
	    /*case TASK_KILLABLE:
	    	rv = "TASK_WAKEKILL, TASK_UNINTERRUPTIBLE";
	    	break;
	    case TASK_STOPPED:
			rv = "TASK_WAKEKILL , __TASK_STOPPED";
			break;
		case _TASK_TRACED:
			rv = "TASK_WAKEKILL , __TASK_TRACED";
			break;
		case TASK_IDLE:
			rv = "TASK_UNINTERRUPTIBLE , TASK_NOLOAD"; 
			break;
		case TASK_NORMAL:
			rv = "TASK_INTERRUPTIBLE , TASK_UNINTERRUPTIBLE";
			break;
		case TASK_REPORT:
			rv = "TASK_RUNNING , TASK_INTERRUPTIBLE , TASK_UNINTERRUPTIBLE , __TASK_STOPPED , __TASK_TRACED , EXIT_ZOMBIE , EXIT_DEAD";
			break;*/
		default:
	    	return "INVALID";
	}
}

module_init(pl_init)
module_exit(pl_exit)
