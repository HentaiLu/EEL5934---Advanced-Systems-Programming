#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h> 
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include<linux/errno.h>  
#include<linux/seq_file.h>
#include<linux/init.h>
#include<linux/device.h>              

#define MYDEV_NAME "mycdrv"

#define ramdisk_size (size_t) (16*PAGE_SIZE) // ramdisk size

//data access directions
#define REGULAR 0
#define REVERSE 1

//NUM_DEVICES defaults to 3 unless specified during insmod
int NUM_DEVICES = 3;

#define CDRV_IOC_MAGIC  'Z'
#define ASP_CHGACCDIR _IO(CDRV_IOC_MAGIC,1)

#define CDRV_IOC_MAXNR 1

static void my_exit(void);
static struct class *foo_class;
int major_no= 0;
int minor_no= 0;

int DIRECTION=0;
struct semaphore blk_sem;
module_param(NUM_DEVICES, int, S_IRUSR);

struct ASP_mycdrv
{
	struct list_head list;
	struct cdev dev;
	char *ramdisk;
	struct semaphore sem;
	int devNo;
};

struct ASP_mycdrv *cdrv_dev_obj;

dev_t dev_num;

char __user * reverse(char __user * rev_buffer, int len)
{
	int i;
	char c;
	for (i=0;i<len/2;i++)
	{
		c=rev_buffer[len-1-i];
		rev_buffer[len-1-i]=rev_buffer[i];
		rev_buffer[i]=c;	
	}
	return rev_buffer;   
}

int open_fn(struct inode *inode, struct file *fp)
{
	struct ASP_mycdrv *dev; 
	dev = container_of(inode->i_cdev, struct ASP_mycdrv, dev);   
	fp->private_data = dev; 
	pr_info("\nOpen : mycdrv%d:\n", dev->devNo);
	return 0;          
}
//File operations
static ssize_t
read_data(struct file *file, char __user * buf, size_t lbuf, loff_t * ppos)
{
	struct ASP_mycdrv *dev = file->private_data;
	int no_byte=0;
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (down_interruptible(&blk_sem))
		return -ERESTARTSYS;
	pr_info("\nDirection of reading: %d\n", DIRECTION);
   
	if(DIRECTION==REGULAR)
	{
		if ((lbuf + *ppos) > ramdisk_size)
		{
			pr_info("Post end of device read\n");
			up(&blk_sem);
			up(&dev->sem);
			return 0;
		}
		no_byte = lbuf - copy_to_user(buf, dev->ramdisk + *ppos, lbuf);
		*ppos += no_byte;
	}

	else if(DIRECTION==REVERSE)
	{
		if ((*ppos -lbuf) < 0)
		{
			pr_info("Read before beginning\n");
			up(&blk_sem);
			up(&dev->sem);
			return 0;
		}
		no_byte = lbuf - copy_to_user(buf, dev->ramdisk + *ppos-lbuf, lbuf);
		*ppos -=no_byte;
		buf=reverse(buf,lbuf);
	}

	else pr_info("Direction Unknown\n");
		pr_info("\n Read function, no_byte=%d, pos=%d\n", no_byte, (int)*ppos);
	up(&blk_sem);
	up(&dev->sem);
	return no_byte; 
}

static ssize_t
write_data(struct file *file, const char __user * buf, size_t lbuf,loff_t * ppos)
{
	int no_byte=0,i;
	char * buffer;
	struct ASP_mycdrv *dev = file->private_data;
	mm_segment_t old_fs;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	buffer=kmalloc((lbuf)*sizeof(char), GFP_KERNEL);   
	if (down_interruptible(&blk_sem))
		return -ERESTARTSYS;
	pr_info("\nFunction is writing in the direction: %d\n", DIRECTION);
	if(DIRECTION==REGULAR)
   	{
		if ((lbuf + *ppos) > ramdisk_size)
		{
			pr_info("Post end of device write\n");
			up(&blk_sem);
			up(&dev->sem);
			return 0;
		}
		no_byte = lbuf - copy_from_user(dev->ramdisk + *ppos, buf, lbuf);
		*ppos += no_byte;
	}

	else if(DIRECTION==REVERSE)
	{
		if ((*ppos -lbuf) < 0)
		{
			pr_info("Write before beginning\n");
			up(&blk_sem);
			up(&dev->sem);
			return 0;
		}
     
		for(i=0;i<lbuf;i++)
			buffer[i]=buf[lbuf-1-i];       
		buf=buffer;
		printk("\n WRITING function output, %s \n", buf);      
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		no_byte = lbuf - copy_from_user(dev->ramdisk + *ppos-lbuf, buf, lbuf);
		set_fs(old_fs);
		*ppos -=no_byte;
	}

	else pr_info("Direction Unknown!\n");
		pr_info("\nWrite function, no_byte=%d, pos=%d\n", no_byte, (int)*ppos);
	kfree(buffer);
	up(&blk_sem);
	up(&dev->sem);
	return no_byte;
}

static int release_fn(struct inode *inode, struct file *file)
{
	struct ASP_mycdrv *dev = file->private_data;
	pr_info("Close: mycdrv%d:\n\n", dev->devNo);
	return 0;
}

static loff_t device_lseek(struct file *file, loff_t offset, int orig) 
{
	struct ASP_mycdrv *dev = file->private_data;
	loff_t position;
	if(down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	switch (orig)
	{
		case SEEK_SET:
			position = offset;
			break;
		case SEEK_CUR:
			position = file->f_pos + offset;
			break;
		case SEEK_END:
			position = ramdisk_size + offset;
			break;
		default:
			return -EINVAL;
	}
	if(position<ramdisk_size)
	{
		position = position;
	}
	else position = ramdisk_size;
	if(position>=0)
	{
		position=position;
	}
	else position=0;
	file->f_pos = position;
	pr_info("Seek position =%ld\n", (long)position);
	up(&dev->sem);
	return position;
}

long device_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	int err_no = 0;
	int retval = 0;
	if(_IOC_TYPE(cmd) != CDRV_IOC_MAGIC)
		return -ENOTTY;
	if(_IOC_NR(cmd) > CDRV_IOC_MAXNR)
		return -ENOTTY;
	if(_IOC_DIR(cmd) & _IOC_READ)
		err_no = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		err_no =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if(err_no)
		return -EFAULT;
   
	switch(cmd)
	{
		case ASP_CHGACCDIR:
			if (! capable (CAP_SYS_ADMIN))
				return -EPERM;
			if (down_interruptible(&blk_sem))
				return -ERESTARTSYS;
			retval=DIRECTION;
			DIRECTION=arg;
			up(&blk_sem);
				return retval;
		default:  
		return -ENOTTY;
	}
}

struct file_operations fops = 
{ 
	.owner = THIS_MODULE,
	.open = open_fn,
	.release = release_fn,
	.write = write_data,
	.read = read_data,
	.llseek=device_lseek,
	.unlocked_ioctl = device_ioctl
};

static void device_setup_cdev(struct ASP_mycdrv *dev, int index)
{
	int errno, devno = MKDEV(major_no, minor_no + index);   
	cdev_init(&dev->dev, &fops);
	dev->dev.owner = THIS_MODULE;
	dev->dev.ops = &fops;
	errno = cdev_add (&dev->dev, devno, 1);
	if (errno)
		printk(KERN_NOTICE "Error %d to add %d", errno, index);
}
//Initialize Driver
static int my_init(void)
{
	int result, i;
	dev_t dev = 0;
	LIST_HEAD(head);
	printk("Loading Character Driver:\n"); 

	if (major_no)
	{
		dev = MKDEV(major_no, minor_no);
		result = register_chrdev_region(dev, NUM_DEVICES, MYDEV_NAME);
   	}
	else
	{
		result = alloc_chrdev_region(&dev, minor_no, NUM_DEVICES,MYDEV_NAME);
		major_no = MAJOR(dev);
	}
	if (result < 0)
	{
		printk(KERN_WARNING "Unable to extract major number %d\n", major_no);
		return result;
	}
	printk("Module registered with major number :%d\n", major_no);
	cdrv_dev_obj = kmalloc(NUM_DEVICES*sizeof(struct ASP_mycdrv), GFP_KERNEL);
	if (!cdrv_dev_obj)
	{
		result = -ENOMEM;
		printk("FAILURE:MALLOC FUNC\n");  
	}
	memset(cdrv_dev_obj, 0, NUM_DEVICES * sizeof(struct ASP_mycdrv));

	foo_class = class_create(THIS_MODULE, "my_class");
	sema_init(&blk_sem, 1);
	for (i = 0; i <  NUM_DEVICES; i++)
	{
		cdrv_dev_obj[i].ramdisk = kmalloc(ramdisk_size, GFP_KERNEL);
		cdrv_dev_obj[i].devNo=i;
		sema_init(&cdrv_dev_obj[i].sem, 1);
		device_setup_cdev(&cdrv_dev_obj[i], i);
		device_create(foo_class, NULL, MKDEV(MAJOR(dev), MINOR(dev) + i) , NULL, "mycdrv%d", i);
		list_add (&cdrv_dev_obj[i].list ,&head) ;
		printk("set up the %dth device.\n",i);
	}
	return 0;
 }
//Unload Driver
static void my_exit(void)
{
	int i;
	dev_t devno = MKDEV(major_no, minor_no);

	if (cdrv_dev_obj)
	{
		for (i = 0; i < NUM_DEVICES; i++)
		{
			kfree(cdrv_dev_obj[i].ramdisk);
			cdev_del(&cdrv_dev_obj[i].dev);
			device_destroy(foo_class, MKDEV(MAJOR(devno), MINOR(devno) + i));
		}
	kfree(cdrv_dev_obj);
	class_destroy(foo_class);
	}
	unregister_chrdev_region(devno, NUM_DEVICES);
	printk("Unloaded Character Driver\n");
}

module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("USER");
MODULE_LICENSE("GPL v2"); 
