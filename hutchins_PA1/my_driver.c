#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define BUFFER_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nathan Hutchins");
static char buffer[BUFFER_SIZE];

// Function Declerations
static int device_open(struct inode *inode, struct file *file);
int my_release(struct inode *inode, struct file *file);
static ssize_t device_read(struct file *file, char *buf, size_t count, loff_t *offp);
static ssize_t device_write(struct file *file, const char *buf, size_t count, loff_t *offp);
loff_t my_seek(struct file *filep, loff_t offset, int whence);

// Jump Table
static struct file_operations device_fops = {
	.owner = THIS_MODULE,
	.write = device_write,
	.open = device_open,
	.read = device_read,
	.release = my_release,
	.llseek = my_seek
};

// Define my init and exit
static int reg_init(void){
	// Register driver with major number 
	if (register_chrdev(240, "simple_character_device", &device_fops) < 0){
		printk(KERN_ALERT "Register Failed\n");
		return -1;
	}

	printk(KERN_ALERT "Init Allocating Memory");
	return 0;
}

static void reg_exit(void){
	unregister_chrdev(240, "simple_character_device");
	printk(KERN_ALERT "Deregister Simple Character Device");
}

// Open the device
static int device_open(struct inode *inode, struct file *file){
	printk(KERN_ALERT "Open Simple Character Device.\n");
	return 0;
}

// Close the device
int my_release(struct inode *inode, struct file *file){
	printk(KERN_ALERT "Close Simple Character Device.\n");
	return 0;
}

static ssize_t device_write(struct file *file, const char *buf, size_t count, loff_t *offp){
	// Copy the user input to the buffer (this return should be 0 on success)
	int bytes_read = count - copy_from_user(buffer + *offp, buf, count);

	// Move the pointer to the end of the written content
	*offp += bytes_read;
	printk(KERN_ALERT "Wrote %d bytes from user.\n", bytes_read);
	return bytes_read;
}

static ssize_t device_read(struct file *file, char *buf, size_t count, loff_t *offp){
	int bytes_read;
	int total_bytes;
	int remaining_bytes;

	total_bytes = BUFFER_SIZE - *offp;

	// See how many bytes we need to read from
	if(total_bytes > count){
		remaining_bytes = count;
	}
	else{
		remaining_bytes = total_bytes;
	}

	// Copy the bytes to users and update the pointer
	bytes_read = remaining_bytes - copy_to_user(buf, buffer + *offp, remaining_bytes);
	*offp += bytes_read;

	// Display how many bytes we read
	printk(KERN_ALERT "Read %d bytes to user.\n", bytes_read);

	return bytes_read;
}

loff_t my_seek(struct file *filep, loff_t offset, int whence){
	loff_t position = 0;
	// Check what condition we are in
	if (whence == 0){
		position = offset;
	}
 	else if (whence == 1){
		position = filep->f_pos + offset;
	}
	else if (whence == 2){
		position = BUFFER_SIZE - offset;
	}
	else{
		return -EINVAL;
	}

	// Check if safe to update the file pointer
	if (position >= 0 && position < BUFFER_SIZE){
		filep->f_pos = position;
		return position;
	}

	// Out of range. Return original pointer position
	printk(KERN_ALERT "Trying to seek out of range ... returning orignal pointer.\n");
	return filep->f_pos;
}

module_init(reg_init);
module_exit(reg_exit);