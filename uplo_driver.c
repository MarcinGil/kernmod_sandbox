
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/ctype.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marcin Gil");
MODULE_DESCRIPTION("Convert upper/lowercase");


#define BUFFER_MAX 512
static int buffer_length = BUFFER_MAX;

module_param(buffer_length, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(buffer_length, "Memory buffer length for string ops.");

static char* UPLO_UPCASE_NAME = "to_upper_case";
static char* UPLO_LOCASE_NAME = "to_lower_case";
static char* UPLO_PROC_NAME = "uplo_driver";

struct proc_dir_entry *uplo_upcase_proc;
struct proc_dir_entry *uplo_locase_proc;

#define UPLO_MAJOR 0 
#define UPLO_UPCASE_MINOR 1
#define UPLO_LOCASE_MINOR 2

static int uplo_upcase_major = 0;
static int uplo_locase_major = 0;

static int uplo_upcase_data_size = 0;
static char uplo_upcase_buffer[BUFFER_MAX];

static int uplo_locase_data_size = 0;
static char uplo_locase_buffer[BUFFER_MAX];

static ssize_t upcase_read(
	struct file* pfile,
	char* buffer,
	size_t length,
	loff_t* offset)
{
	static int subsequent_read = 0;
	int to_copy = 0;

	if (subsequent_read || *offset >= uplo_upcase_data_size) {
		subsequent_read = 0;
		return 0;
	}

	++subsequent_read;

	to_copy = uplo_upcase_data_size - *offset;
	if (length < to_copy)
		to_copy = length;

	if (copy_to_user(buffer, uplo_upcase_buffer, to_copy)) {
		return -EFAULT;
	}

	*offset = *offset + to_copy;
	return to_copy;
}

static ssize_t upcase_write(
	struct file* pfile,
	const char* buffer,
	size_t length,
	loff_t* offset)
{
	int i = 0;

	if (length >= BUFFER_MAX) {
		uplo_upcase_data_size = BUFFER_MAX;
	} else {
		uplo_upcase_data_size = length;
	}

	if (copy_from_user(uplo_upcase_buffer, buffer, uplo_upcase_data_size)) {
		return - EFAULT;
	}

	for (i = 0; i < uplo_upcase_data_size; ++i) {
		uplo_upcase_buffer[i] = toupper(uplo_upcase_buffer[i]);
	}	
	printk(KERN_INFO "upcase_write: wrote %i bytes\n", uplo_upcase_data_size);

	return uplo_upcase_data_size;
}

int
uplo_open(struct inode* inode, struct file* file)
{
	try_module_get(THIS_MODULE);
	return 0;
}

int
uplo_close(struct inode* inode, struct file* file)
{
	module_put(THIS_MODULE);
	return 0;
}

static int
uplo_permission(struct inode* inode, int mask, unsigned int flags) 
{
	return 0;
}

static struct file_operations uplo_upcase_file_ops = {
	.read = upcase_read,
	.write = upcase_write,
	.open = uplo_open,
	.release = uplo_close,
};

/*
static struct inode_operations uplo_inode_ops = {
	.permission = uplo_permission,
};
*/

int
register_device(int major, char* name, struct file_operations* filp)
{
	int retcode;
	retcode = register_chrdev(major, name, filp);
	if (retcode < 0) {
		printk(KERN_ALERT "Registering device %s failed\n", name);
		return major;
	}

	printk(KERN_INFO "Assigned major number %d for %s. To talk to\n", retcode, name);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", name, retcode);

	return retcode;
}

int __init uplo_case_init(void)
{
/*	if ((uplo_upcase_proc = proc_create(UPLO_PROC_NAME, S_IFREG | S_IRUGO | S_IWUSR, NULL, &uplo_upcase_file_ops)) != NULL) {
		uplo_upcase_proc->proc_iops = &uplo_inode_ops; 
	}
*/	
	uplo_upcase_major = register_device(UPLO_MAJOR, UPLO_UPCASE_NAME, &uplo_upcase_file_ops);

	printk(KERN_INFO "UpLoCase initialized with %i buffer\n", buffer_length);
	return 0;
}

void __exit uplo_case_exit(void)
{
	unregister_chrdev(uplo_upcase_major, UPLO_UPCASE_NAME);

/*	remove_proc_entry(UPLO_PROC_NAME, NULL);*/

	printk(KERN_INFO "UpLoCase exiting\n");
	return;
}

module_init(uplo_case_init);
module_exit(uplo_case_exit);
