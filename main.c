
/*
	PROJECT NAME : LCD16X2 I2C CLIENT CHARATER DEVICE DRIVER

	DEVELOPED BY :

*/
#include<linux/device.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h> 
#include<linux/slab.h> 
#include<linux/cdev.h>
#include<linux/gpio.h> 
#include<linux/init.h>
#include<linux/i2c.h>
#include<linux/kernel.h>
#include<linux/delay.h>

//#include"lcd16x2_i2c.h"


//-----------------------------------------------------------------//
//Declaration Specific to I2C client device
#define PIN_RESTS           0x01
#define PIN_RW              0X02
#define PIN_EN              0X04
#define PIN_BK_LIGHT        0X08


//----------------------------------------------------------------//
//Declaration Specific to LCD16x2 Functionality
#define LCD_CMD_CLEAR_DISP              0X01
#define LCD_CMD_RETURN_TO_HOME          0X02
#define LCD_CMD_MOVE_DISP_LEFT          0X18
#define LCD_CMD_MOVE_DISP_RIGHT         0X1C

#define LCD_CMD_CURSOR_SHIFT            0X10
#define LCD_CMD_CLEAR_DISP              0X01

#define LCD_CMD_CURSOR_ON               0X02
#define LCD_CMD_CURSOR_Off              0X00

#define LCD_CMD_2LINE                   0X08
#define LCD_CMD_1LINE                   0X00



/*
    DETAILS : 
        BBB : I2C2_SCL (pin_no 19) = SCL
        BBB : I2C2_SDA (pin_no 20) = SDA
        BBB : DGND (pin_no 01) = GND
        BBB : SYS_5V (pin_no 07) = VCC (NOTE : connect only if No External 5V supply)
*/

#define ERROR -1
#define I2C_BUS_AVAILABLE	2
#define SLAVE_DEVICE_NAME	"lcd16x2_i2c"
#define LCD16X2_SLAVE_ADDR	0x27

static int major = 250;
static dev_t devno;
static struct class *pclass;
static struct cdev lcd16x2_i2c_cdev;

//structure declaration
static struct i2c_adapter *lcd16x2_i2c_adapter  =   NULL; //i2c Adapter Structure
static struct i2c_client *lcd16x2_i2c_client = NULL; //i2c Client structure (in our case it is LCD16x2_i2c) 

static struct file_operations fops;

//FUNCTION DECLARATIONS
static int lcd16x2_init_4bit_mode(void);
static int lcd16x2_i2c_cmd_send(uint8_t cmd);
static int lcd16x2_i2c_data_send(uint8_t data);
//static int I2C_Write(unsigned char *buf, unsigned int len);




//----------------------Function Definatipons--------------------------------------------//
int lcd16x2_init_4bit_mode(void)
{
    printk(KERN_INFO"%s lcd16x2_init_4bit() called !\n",THIS_MODULE->name);
    mdelay(50); //20msec delay after turn on of LCD
    lcd16x2_i2c_cmd_send(0x3);
    
    mdelay(5); //5msec delay
    lcd16x2_i2c_cmd_send(0x3);

    mdelay(5); //5msec delay
    lcd16x2_i2c_cmd_send(0x3);

    mdelay(5); //5msec delay
    lcd16x2_i2c_cmd_send(0x2);


//for 4-bit mode = 0x28 is given to the LCD
    mdelay(5); //5msec delay
    lcd16x2_i2c_cmd_send(0x2);

    mdelay(1); //5msec delay
    lcd16x2_i2c_cmd_send(0x8);


//for disp ON, and cuesor blink = 0x0f given to the LCD
    mdelay(5); //5msec delay
    lcd16x2_i2c_cmd_send(0x0);

    mdelay(1); //5msec delay
    lcd16x2_i2c_cmd_send(0xf);

// for disp clear = 0x01
    mdelay(5);
    lcd16x2_i2c_cmd_send(0x0);
    mdelay(1);
    lcd16x2_i2c_cmd_send(0x1);
    mdelay(5);


    return 0;
}

int lcd16x2_i2c_cmd_send(uint8_t cmd)
{

    uint8_t cmd_u, cmd_l, cmd_t[4], ret;
    
    cmd_u = (cmd & 0xf0);
    cmd_l = ((cmd << 4) & 0xf0);

    cmd_t[0] = (cmd_u | PIN_BK_LIGHT | PIN_EN );
    cmd_t[1] = (cmd_u | PIN_BK_LIGHT | PIN_RESTS);
    cmd_t[2] = (cmd_l | PIN_BK_LIGHT | PIN_EN );
    cmd_t[3] = (cmd_l | PIN_BK_LIGHT | PIN_RESTS);
    
//send data to i2c client device
    //I2C_Write(data_t,4);
    ret = i2c_master_send(lcd16x2_i2c_client, cmd_t, 4);
    if (ret < 0)    
    {
        //ERROR
        printk(KERN_INFO"i2c_master_send() Failed %d:%s:%s",__LINE__,__func__,__FILE__);
    }

    return 0;
}

int lcd16x2_i2c_data_send(uint8_t data)
{
    
    uint8_t data_u, data_l, data_t[4], ret;
    
    data_u = (data & 0xf0);
    data_l = ((data << 4) & 0xf0);

    data_t[0] = (data_u | PIN_BK_LIGHT | PIN_EN | PIN_RESTS);
    data_t[1] = (data_u | PIN_BK_LIGHT | PIN_RESTS);
    data_t[2] = (data_l | PIN_BK_LIGHT | PIN_EN | PIN_RESTS);
    data_t[3] = (data_l | PIN_BK_LIGHT | PIN_RESTS);
    

//send data to i2c client device
    ret = i2c_master_send(lcd16x2_i2c_client, data_t, 4);
    if (ret < 0)    
    {
        //ERROR
        printk(KERN_INFO"i2c_master_send() Failed %d:%s:%s",__LINE__,__func__,__FILE__);
    }
    
    return 0;
}



static int lcd16x2_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret, minor, temp, i;
	struct device *pdevice;
    char str[] = "Hello World";

	printk(KERN_INFO"lcd16x2_i2c_probe() invoked %d:%s:%s\n",__LINE__,__func__,__FILE__);    
	devno = MKDEV(major, 0);//major nymber is choosen dynamically

    ret = alloc_chrdev_region(&devno, 0, 1,"lcd16x2_i2c");
    if(ret < 0)
    {
        printk(KERN_INFO"%s : alloc_chardev_region() failed!\n",__FILE__);
        return -1;
    }

	major = MAJOR(devno);
	minor = MINOR(devno);
	printk(KERN_INFO "%s: alloc_chrdev_region() allocated dev number (%d/%d).\n", THIS_MODULE->name, major, minor);

	//Create Device class
	pclass = class_create(THIS_MODULE, "lcd16x2_i2c");
    if(IS_ERR(pclass))
    {
        printk(KERN_INFO"%s : class_create() failed!\n",__FILE__);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
	printk(KERN_INFO"%d:%s : class_create() : Class created!\n",__LINE__,__FILE__);

    //create device file
    pdevice = device_create(pclass, NULL, devno, NULL,"lcd16x2_i2c %d", 0);
    if(IS_ERR(pdevice))
    {
        printk(KERN_INFO"%s : device_create() failed!\n",__FILE__);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO"%s : device_create() Successfull!\n",__FILE__);

	//Initialize struct cdev and add it inot the kernel
	// initialize struct cdev and add it into the kernel
	cdev_init(&lcd16x2_i2c_cdev, &fops);
	ret = cdev_add(&lcd16x2_i2c_cdev, devno, 1);
	if(ret < 0) {
		printk(KERN_INFO "%s: cdev_add() failed (%d).\n", THIS_MODULE->name, ret);
		device_destroy(pclass, devno);
		class_destroy(pclass);
        unregister_chrdev_region(devno, 1);

	}
	printk(KERN_INFO "%s: cdev_add() added cdev into kernel.\n", THIS_MODULE->name);
  

//--------------------------------------------------------------------------------//
    temp = lcd16x2_init_4bit_mode();
    printk(KERN_INFO"lcd16x2_init_4bit_mode() invoked %d:%s:%s",__LINE__,__func__,__FILE__);

    temp = sizeof(str);
    for(i=0; i < (temp-1); i++)
    {
        lcd16x2_i2c_data_send(str[i]);
    }

    printk(KERN_INFO"%s:%d:lcd16x2_i2c_data_send()\n",THIS_MODULE->name,__LINE__);


    return 0;
}



static int lcd16x2_i2c_remove(struct i2c_client *client)
{
	printk(KERN_INFO"lcd16x2_i2c_remove() invoked %d:%s:%s\n",__LINE__,__func__,__FILE__);

	//Delete cdev
	cdev_del(&lcd16x2_i2c_cdev);
	printk(KERN_INFO"%d:%s : cdev_del() destroy dev file !\n",__LINE__,__FILE__);

	//Destroy the Device File
    device_destroy(pclass, devno);
    printk(KERN_INFO"%s : device_destroy() destroy dev file !\n",__FILE__);

    //Destroy the Device Class
    class_destroy(pclass);
    printk(KERN_INFO"%s : class_destroy() destroy dev class !\n",__FILE__);

	//Released Device Number
    unregister_chrdev_region(devno, 1);
    printk(KERN_INFO"%s : unregister_chardev_reg() released dev num !\n",__FILE__);


    return 0;
}


/*
	STRUCTURE THAT HAS SLAVE DEVICE ID
*/
static const struct i2c_device_id lcd16x2_i2c_id[] = {
        { SLAVE_DEVICE_NAME, 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, lcd16x2_i2c_id);


/*
	I2C DRIVER STRUCTURE THAT HAS TO BE ADDED IN TO THE LINUX KERNEL
*/
static struct i2c_driver lcd16x2_i2c_driver = {
	.driver = {
		.name	= SLAVE_DEVICE_NAME,
		.owner	= THIS_MODULE,
	},
	.probe = lcd16x2_i2c_probe,
	.remove = lcd16x2_i2c_remove,
	.id_table = lcd16x2_i2c_id,
};


/*
I2C BOARD INFO STRUCTURE
*/
static struct i2c_board_info lcd16x2_i2c_board_info = {
        I2C_BOARD_INFO(SLAVE_DEVICE_NAME, LCD16X2_SLAVE_ADDR)
    };


static int __init lcd16x2_i2c_init(void)
{
	int ret = -1;

	//i2c driver
    //get i2c bus(adaptor)
	lcd16x2_i2c_adapter     = i2c_get_adapter(I2C_BUS_AVAILABLE);
    printk(KERN_INFO"i2c_get_adapter() invoked %d:%s:%s\n",__LINE__,__func__,__FILE__);
  
  	if( lcd16x2_i2c_adapter  != NULL )
  	{
    	lcd16x2_i2c_client = i2c_new_device(lcd16x2_i2c_adapter, &lcd16x2_i2c_board_info);
        printk(KERN_INFO"i2c_new_device() invoked %d:%s:%s\n",__LINE__,__func__,__FILE__);
    	
        if( lcd16x2_i2c_client != NULL )
    	{
        	i2c_add_driver(&lcd16x2_i2c_driver);
            printk(KERN_INFO"i2c_add_driver() invoked %d:%s:%s\n",__LINE__,__func__,__FILE__);

        	ret = 0;
      	}
      
    	i2c_put_adapter(lcd16x2_i2c_adapter);
        printk(KERN_INFO"i2c_put_adapter() invoked %d:%s:%s\n",__LINE__,__func__,__FILE__);
  	}
  
  	pr_info("Driver Added!!!\n");
  	return ret; 

}


static void __exit lcd16x2_i2c_exit(void)
{
    printk(KERN_INFO"Exit module invoked %d:%s:%s\n",__LINE__,__func__,__FILE__);

	//Removed I2C device
	i2c_unregister_device(lcd16x2_i2c_client);

    i2c_del_driver(&lcd16x2_i2c_driver);
	printk(KERN_INFO"i2c_del_driver() invoked %d:%s:%s\n",__LINE__,__func__,__FILE__);


}

module_init(lcd16x2_i2c_init);
module_exit(lcd16x2_i2c_exit);
MODULE_LICENSE("GPL");



/*
||----------------------->>> REFERENCES <<<-------------------------------||
| * kernel.org/doc/Documentation/i2c/writing-clients
| * docs.kernel.org/i2c/index.html
|
|_________________________________________________________________________||
*/