
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Marcin Gil");
MODULE_DESCRIPTION("Convert upper/lowercase");


#define BUFFER_MAX 512
static int buffer_length = BUFFER_MAX;

module_param(buffer_length, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(buffer_length, "Memory buffer length for string ops.");

static char* UPLO_UPCASE_NAME = "uplo_upcase";
static char* UPLO_LOCASE_NAME = "uplo_locase";

struct proc_dir_entry *uplo_upcase_proc;
struct proc_dir_entry *uplo_locase_proc;

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

	if (subsequent_read) {
		subsequent_read = 0;
		return 0;
	}

	if (copy_to_user(buffer, uplo_upcase_buffer, uplo_upcase_data_size)) {
		return -EFAULT;
	}

	return uplo_upcase_data_size;
}

static ssize_t upcase_write(
	struct file* pfile,
	const char* buffer,
	size_t length,
	loff_t* offset)
{
	if (length >= BUFFER_MAX) {
		uplo_upcase_data_size = BUFFER_MAX;
	} else {
		uplo_upcase_data_size = length;
	}

	if (copy_from_user(uplo_upcase_buffer, buffer, uplo_upcase_data_size)) {
		return - EFAULT;
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

static struct file_operations uplo_upcase_file = {
	.read = upcase_read,
	.write = upcase_write,
	.open = uplo_open,
	.release = uplo_close,
};

static struct inode_operations uplo_inode_ops = {
	.permission = uplo_permission,
};

void check_mem(void);


int __init uplo_case_init(void)
{
	check_mem();

	if ((uplo_upcase_proc = proc_create(UPLO_UPCASE_NAME, S_IFREG | S_IRUGO | S_IWUSR, NULL, &uplo_upcase_file)) != NULL) {
		uplo_upcase_proc->proc_iops = &uplo_inode_ops;
	}

	printk(KERN_INFO "UpLoCase initialized with %i buffer\n", buffer_length);
	return 0;
}

void __exit uplo_case_exit(void)
{
	remove_proc_entry(UPLO_UPCASE_NAME, NULL);

	printk(KERN_INFO "UpLoCase exiting\n");
	return;
}

void check_mem(void)
{
	if (buffer_length > BUFFER_MAX) {
		buffer_length = BUFFER_MAX;
	}
}

module_init(uplo_case_init);
module_exit(uplo_case_exit);
