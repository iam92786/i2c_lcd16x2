/***************************************************************************//**
		PROJECT : I2C CLIENT DEVICE DRIVER FOR MPU6050
		AUTHOR : ARSHAD HUSSAIN

* *******************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
 
#define I2C_BUS_AVAILABLE   (          2 )              // I2C Bus available in our Beaglebone Black
#define SLAVE_DEVICE_NAME   ( "MPU6050"  )              // Device and Driver Name
#define MPU6050_SLAVE_ADDR  (       0x68 )              //  MPU6050 Slave Address
 
static struct i2c_adapter *i2c_adapter     = NULL;  // I2C Adapter Structure
static struct i2c_client  *i2c_client = NULL;  // I2C Cient Structure (In our case it is OLED)
 
#define DRIVER_CLASS "MPU6050class"
#define DRIVER_NAME "mpu6050"
 
#define MPU6050_WHO_AM_I_REG    0x75
#define MAXLEN	6
  
// buffer size
unsigned char buffer[MAXLEN];

static struct file_operations fops;

/* Variables for Device and Deviceclass*/
static int major=250;
static dev_t devno;
static struct class *pclass;
static struct cdev MPU6050_cdev;

/*
** This function writes the data into the I2C client
**
**  Arguments:
**      buff -> buffer to be sent
**      len  -> Length of the data
**   
*/
static int I2C_Write(unsigned char *buf, unsigned int len)
{
	printk("i2c_write called\n");
  /*
  ** Sending Start condition, Slave address with R/W bit, 
  ** ACK/NACK and Stop condtions will be handled internally.
  */ 
  int ret = i2c_master_send(i2c_client, buf, len);
  
  return ret;
}
 
/*
** This function reads one byte of the data from the I2C client
**
**  Arguments:
**      out_buff -> buffer wherer the data to be copied
**      len      -> Length of the data to be read
** 
*/
static int I2C_Read(unsigned char *out_buf, unsigned int len)
{
  /*
  ** Sending Start condition, Slave address with R/W bit, 
  ** ACK/NACK and Stop condtions will be handled internally.
  */ 
  int ret = i2c_master_recv(i2c_client, out_buf, len);
  
  return ret;
}
 
/*
** This function is specific to the SSD_1306 OLED.
** This function sends the command/data to the OLED.
**
**  Arguments:
**      is_cmd -> true = command, flase = data
**      data   -> data to be written
** 
*/
static int MPU6050_open(struct inode *pinode, struct file *pfile) {
	printk(KERN_INFO "%s: driver_open() called.\n", THIS_MODULE->name);
	return 0;
}

static int MPU6050_close(struct inode *pinode, struct file *pfile) {
	printk(KERN_INFO "%s: driver_close() called.\n", THIS_MODULE->name);
	return 0;
}
 static ssize_t MPU6050_write(struct file *pfile, const char __user *ubuf, size_t len, loff_t *poffset) {
 	printk(KERN_INFO "%s: write () called.\n", THIS_MODULE->name);
 	unsigned char writebuf[2];
 	writebuf[0] = 0x6B;
 	writebuf[1] = 0x00;
 	
 	int ret = I2C_Write(writebuf,2);
 	printk("write value ret = %d\n",ret);
 	
 	return ret;
 }
 
  static ssize_t MPU6050_read(struct file *File, char __user *ubuf, size_t count, loff_t *offs) 
  {
	printk("hello");
	int avail_bytes, bytes_to_read, nbytes;
	printk(KERN_INFO "%s: MPU6050_read() called.\n", THIS_MODULE->name);
	
	buffer[0] = 0x3B;
	if(I2C_Write(buffer,1)!=1)
	{
		printk("error to select acc x axix register\n");
	}
	
	
	
	int pBuffer[3];
	int ret;
	
	
	avail_bytes = MAXLEN;
	
	//char buffer[6]={0};
	ret = I2C_Read(buffer,6);	
	printk("read = %d\n",ret);
	printk("read = %s\n",buffer);
	 char buffertest[] = "kiran";	
	nbytes = copy_to_user(ubuf, buffer,6);
	printk("nbytes = %d\n",nbytes);
	
	pBuffer[0] = (int) ( (buffer[0] << 8) |  buffer[1] );
	pBuffer[1] = (int) ( (buffer[2] << 8) |  buffer[3] );
    pBuffer[2] = (int) ( (buffer[4] << 8) |  buffer[5] );

 	printk("acc x %d\n",pBuffer[0]);
	printk("acc y %d\n",pBuffer[1]);
	printk("acc z %d\n",pBuffer[2]);
	
	return nbytes;
}

 /* Map the file operations */
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = MPU6050_open,
	.release = MPU6050_close,
	.write = MPU6050_write,
	.read = MPU6050_read,
};

  /*
** This function getting called when the slave has been found
** Note : This will be called only once when we load the driver.
*/
static int MPU6050_probe(struct i2c_client *client,
                      	 const struct i2c_device_id *id)
{


 //  pr_info("MPU6050 Probed!!!\n"); 
	printk("MyDeviceDriver - Hello Kernel\n");
 unsigned char who_am_i;
//	 int dc;
	
	int ret, minor;
	struct device *pdevice;
	printk(KERN_INFO "%s: MPU6050_probe() called.\n", THIS_MODULE->name);
	
	// allocate device number
	devno = MKDEV(major, 0);
	ret = alloc_chrdev_region(&devno, 0, 1, "MPU6050");
	if(ret < 0) {
		printk(KERN_INFO "%s: alloc_chrdev_region() failed (%d).\n", THIS_MODULE->name, ret);
		goto return_error_label;
	}
	major = MAJOR(devno);
	minor = MINOR(devno);
	printk(KERN_INFO "%s: alloc_chrdev_region() allocated dev number (%d/%d).\n", THIS_MODULE->name, major, minor);
	
	// create device class
	pclass = class_create(THIS_MODULE, "MPU6050_class");
	if(IS_ERR(pclass)) {
		printk(KERN_INFO "%s: class_create() failed.\n", THIS_MODULE->name);
		ret = -1;
		goto unregister_chrdev_region_label;
	}
	printk(KERN_INFO "%s: class_create() created device class.\n", THIS_MODULE->name);

	// create device file
	pdevice = device_create(pclass, NULL, devno, NULL, "MPU6050-");
	if(IS_ERR(pdevice)) {
		printk(KERN_INFO "%s: device_create() failed.\n", THIS_MODULE->name);
		ret = -1;
		goto class_destroy_label;
	}
	printk(KERN_INFO "%s: device_create() created device file.\n", THIS_MODULE->name);
	
	// initialize struct cdev and add it into the kernel
	cdev_init(&MPU6050_cdev, &fops);
	ret = cdev_add(&MPU6050_cdev, devno, 1);
	if(ret < 0) {
		printk(KERN_INFO "%s: cdev_add() failed (%d).\n", THIS_MODULE->name, ret);
		goto device_destroy_label;
	}
	printk(KERN_INFO "%s: cdev_add() added cdev into kernel.\n", THIS_MODULE->name);
  
    printk(KERN_INFO "MPU6050 device detected at address 0x%x\n", client->addr);



	  // Check that the device is actually an MPU6050
    who_am_i = i2c_smbus_read_byte_data(i2c_client, MPU6050_WHO_AM_I_REG);
    printk("who am i %x\n",who_am_i);
    if (who_am_i != 0x68) {
    	printk(KERN_ERR "MPu6050 who am i 0x%x\n", who_am_i);
        printk(KERN_ERR "MPU6050 not detected at address 0x%x\n", i2c_client->addr);
        return -ENODEV;
    }
	printk(KERN_INFO "MPU6050 device detected at address 0x%x\n",i2c_client->addr);
   //  Save I2C client for use in other functions
    //etx_i2c_client_oled = client;

  pr_info("MPU6050 Probed!!!\n");
  
  return 0;
  
 device_destroy_label:
	device_destroy(pclass, devno);
class_destroy_label:
	class_destroy(pclass);
unregister_chrdev_region_label:	
	unregister_chrdev_region(devno, 1);
return_error_label:
	return ret;

}
 
/*
** This function getting called when the slave has been removed
** Note : This will be called only once when we unload the driver.
*/
static int MPU6050_remove(struct i2c_client *client)
{   
 printk(KERN_INFO "%s: MPU6050_remove() called.\n", THIS_MODULE->name);
	
	
	cdev_del(&MPU6050_cdev);
	printk(KERN_INFO "%s: cdev_del() removed cdev from kernel.\n", THIS_MODULE->name);
	
	// destroy the device file
	device_destroy(pclass, devno);
	printk(KERN_INFO "%s: device_destroy() destroyed the device file.\n", THIS_MODULE->name);

	// destroy the device class
	class_destroy(pclass);
	printk(KERN_INFO "%s: class_destroy() destroyed the device class.\n", THIS_MODULE->name);

	// release device number
	unregister_chrdev_region(devno, 1);
	printk(KERN_INFO "%s: unregister_chrdev_region() released dev number.\n", THIS_MODULE->name);

	return 0;
}
 
/*
** Structure that has slave device id
*/
static const struct i2c_device_id MPU6050_DEVICE_id[] = {
  { SLAVE_DEVICE_NAME, 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, MPU6050_DEVICE_id);
 
/*
** I2C driver Structure that has to be added to linux
*/
static struct i2c_driver MPU6050_driver = {
  .driver = {
      .name   = SLAVE_DEVICE_NAME,
      .owner  = THIS_MODULE,
  },
  .probe          = MPU6050_probe,
  .remove         = MPU6050_remove,
  .id_table       = MPU6050_DEVICE_id,
};
 
/*
** I2C Board Info strucutre
*/
static struct i2c_board_info MPU6050_i2c_board_info = {
  I2C_BOARD_INFO(SLAVE_DEVICE_NAME, MPU6050_SLAVE_ADDR)
};
 
/*
** Module Init function
*/
static int __init MPU6050_init(void)
{
  int ret = -1;
  i2c_adapter     = i2c_get_adapter(I2C_BUS_AVAILABLE);
  
  if( i2c_adapter != NULL )
  {
      i2c_client = i2c_new_device(i2c_adapter, &MPU6050_i2c_board_info);
      
      if( i2c_client != NULL )
      {
          i2c_add_driver(&MPU6050_driver);
          ret = 0;
      }
      
      i2c_put_adapter(i2c_adapter);
  }
  
  pr_info("Driver Added!!!\n");
  return ret;
}
 
/*
** Module Exit function
*/
static void __exit MPU6050_exit(void)
{
  
  pr_info("Driver Removed!!!\n");
    i2c_unregister_device(i2c_client);
	  i2c_del_driver(&MPU6050_driver);
}
 
module_init(MPU6050_init);
module_exit(MPU6050_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arshad <iam92786@gmail.com>");
MODULE_DESCRIPTION("i2c based accelerometer device driver");

