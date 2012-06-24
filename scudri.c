/* This device driver make use of memeory as a device and performs read and write functions on it.
   This is a char driver that acts on a memory area as though it were a device.
   This device which we are using is hardware independent it just acts on the piece of memory allocated by the kernel.
   In this device driver we have used functions "myread" and "mywrite".
   In this device driver we are using memory in the form of quanta each quanta can hold 4000 bytes of the memory and we are using qset to represent the   number of quantums.
*/


#include<linux/kdev.h>		//including MKDEV macro;
#include<linux/fs.h>		//including file and other structures
#include<linux/module.h>
#include<linux/init.h>
#include<asm/uaccess.h>		//function to move data
#include<linux/kernel.h>	//container_of() fnctn
#include<linux/cdev.h>		//registry fnctn
#include<linux/slab.h>		//to use the functions kmalloc and kfree


#define major 900
#define minor 0

// This structure is predefined in fs.h which we can use to perform operations on our memory.
struct file_operation mydiv_fops={
		.open=myopen,
		.owner=THIS_MODULE,
		.read=myread,
		.write=mywrite,
		.release=myrelease,
		};


// This structure is used to point at the data.
struct dataset{
		void** data;
		struct dataset* next;
	      };


// This is our main device structure.
struct mydevice{
		struct dataset set;	// store address of first data
		unsigned long size;	// To store the memory occupied
		struct cdev cdev;	//structure which represent file in 
					//kernel internel
		int quanta;		// We have distributed memory into samll chunks using quanta to make our data handling simple.
		int qset;		// current array size. Stores number of current quanta.
		struct semaphore sem;	//to take care of compiler optimization
		
		};

//Prototypes of the functions

static void my_device_setup_cdev(struct mydevice* , int ); 	//This function intializes and adds the device to the system for usage.
int myopen(struct inode * , struct file * );			// To open the device.
int myrelease(struct inode * , struct file * );		 	// To release the device.
int mytrim(struct mydevice *);					// This function trims the memory when required. For example when we over-write with a  short file then it trims rest of the memory.
int myread(struct file *, char __user *, size_t , loff_t * );
int mywrite(struct file *, char __user *, size_t , loff_t * );


//SEPERATE FUNCTION DEFINATIONS

int mytrim(struct mydevice *dev)
{
	struct dataset *next, *dptr;
	int qset = dev->qset;		/* "dev" is not-null */
	int i;
	for (dptr = dev->data; dptr; dptr = next) 
	{ /* all the list items */
		if (dptr->data) 
		{
			for (i = 0; i < qset; i++) kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	dev->size = 0;
	dev->quantum = 4000;
	dev->qset =1000;
	dev->data = NULL;
	return 0;
}
int myopen(struct inode *inode,struct file * filp)
{
	struct mydev *dev; 		// device information
	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev; 	// for other methods

	// now trim to 0 the length of the device if open was write-only 

	if ( (filp->f_flags & O_ACCMODE) = = O_WRONLY) mytrim(dev); 	//ignore errors

	/* O_ACCMODE and O_WRONGLY are macros defined in fcntl.h O_WRONLY stands for write only access.
	   O_ACCMODE when bitwise ANDed with file status flag value it produces a value representing the
	   file access mode.*/

	return 0;			// success
}

static void my_device_setup_cdev(struct mydevice *dev , int index)
{
	int err;
	dev_t devno = MKDEV(major, minor + index);
	cdev_init(&dev->cdev, &scull_fops);		//initializing device

	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;

	err = cdev_add (&dev->cdev, devno, 1);		//adding device so that it's entry get defined in proc/devices

	//if fail
	if (err)
	printk(KERN_NOTICE "Error %d adding scull%d", err, index);
}

int myread(struct file *filp , char __user *buf , size_t size, loff_t *f_pos)
{
	mydevice *dev;
	dev=filp->private_data;

	int quant=dev->quanta,qset=dev->qset;
	long int setsize=quant*qset,dsize=dev->size;
	loff_t pos=*f_pos;

	if(pos>=dsize) retorn 0;
	if(pos+size>dsize) size=dsize-pos;

	/* Now we have to reachto the pos to read data from there. As data is being written 
	   in the form of qset and quantum we have to reachto the proper quantum.	*/

	int setpos=pos/setsize;
	int rest=pos%setsize;
	int qpos=rest/quant;
	int qbit=rest%quant;

	int temp=0;

	struct dataset dset=dev->set;

//reach to setpos
	while(setpos--) dset=dset.next;

//now we have the dataset that contains our pos, we just now want to reach exactly to
//that position.

	char *data=dset[qpos]+qbit;

	if(size > quant-qbit) size=quant-qbit;

	if(copy_to_user(buf,data,size))
	{
		return 0;
	}

	*f_pos+=size;
	return size;
}

int mywrite(struct file *filp , char __user *buf , size_t size, loff_t *f_pos)
{
	mydevice *dev;
	dev=filp->private_data;

	int quant=dev->quanta,qset=dev->qset;
	long int setsize=quant*qset,dsize=dev->size;
	loff_t pos=*f_pos;

	if(pos>=dsize) retorn 0;
	if(pos+size>dsize) size=dsize-pos;

	/* Now we have to reach to the pos to write data from there.	*/

	int setpos=pos/setsize;
	int rest=pos%setsize;
	int qpos=rest/quant;
	int qbit=rest%quant;

	int temp=0;

	struct dataset dset=dev->set;

//reach to setpos
	while(setpos--) dset=dset.next;

//now we have the dataset that contains our pos, we just now want to reach exactly to
//that position and also we have to assign memory.

	if(!dset->data)
	{
		dset->data = kmalloc(qset*sizeof(char*),GFP_KERNEL);
		memset(dset->data, 0, qset * sizeof(char *));		//Initialises to NULL
	}

	if(!dset->data[qpos]) dset->data[qpos]=kmalloc(quant,GFP_KERNEL);


	if(size > quant-qbit) size=quant-qbit;

	if(copy_from_user(dset->data[qpos]+qbit,buf,size))
	{
		return 0;
	}

	*f_pos+=size;
	if(*f_pos > dev->size) dev->size+=(long) *f_pos;

	return size;
}
