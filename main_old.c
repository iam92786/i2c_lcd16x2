
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

#include"lcd16x2_i2c.h"

/*
    DETAILS : 
        BBB : I2C2_SCL (pin_no 19) = SCL
        BBB : I2C2_SDA (pin_no 20) = SDA
        BBB : DGND (pin_no 01) = GND
        BBB : SYS_5V (pin_no 07) = VCC (NOTE : connect only if No External 5V supply)
*/
#define ERROR -1
#define I2C_BUS_AVAILABLE 2
#define SLAVE_DEVICE_NAME "lcd16x2_i2c_client"
#define LCD16X2_SLAVE_ADDR  0x27

static int major = 250;
static dev_t devno;
static struct class *pclass;

//structure declaration
static struct i2c_adapter *lcd16x2_i2c_adapter  =   NULL; //i2c Adapter Structure
static struct i2c_client *lcd16x2_i2c_client = NULL; //i2c Client structure (in our case it is LCD16x2_i2c) 

//FUNCTION DECLARATIONS
int lcd16x2_i2c_init(void);
int lcd16x2_i2c_cmd_send(uint8_t cmd);
int lcd16x2_i2c_data_send(uint8_t data);


//----------------------Function Definatipons--------------------------------------------//
int lcd16x2_i2c_init(void)
{

    return 0;
}

int lcd16x2_i2c_cmd_send(uint8_t cmd)
{

    uint8_t cmd_u, cmd_l, cmd_t[4];
    
    cmd_u = (cmd & 0xf0);
    cmd_l = ((cmd << 4) & 0xf0);

    cmd_t[0] = (cmd_u | PIN_BK_LIGHT | PIN_EN );
    cmd_t[1] = (cmd_u | PIN_BK_LIGHT | PIN_RESTS);
    cmd_t[2] = (cmd_l | PIN_BK_LIGHT | PIN_EN );
    cmd_t[3] = (cmd_l | PIN_BK_LIGHT | PIN_RESTS);
    
//send data to i2c client device


    return 0;
}

int lcd16x2_i2c_data_send(uint8_t data)
{
    
    uint8_t data_u, data_l, data_t[4];
    
    data_u = (data & 0xf0);
    data_l = ((data << 4) & 0xf0);

    data_t[0] = (data_u | PIN_BK_LIGHT | PIN_EN | PIN_RESTS);
    data_t[1] = (data_u | PIN_BK_LIGHT | PIN_RESTS);
    data_t[2] = (data_l | PIN_BK_LIGHT | PIN_EN | PIN_RESTS);
    data_t[3] = (data_l | PIN_BK_LIGHT | PIN_RESTS);
    

//send data to i2c client device


    return 0;
}


static int lcd16x2_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk(KERN_INFO"lcd16x2_i2c_probe() invoked %d:%s:%s\n",__LINE__,__func__,__FILE__);
    return 0;
}

static int lcd16x2_i2c_remove(struct i2c_client *client)
{
	printk(KERN_INFO"lcd16x2_i2c_remove() invoked %d:%s:%s\n",__LINE__,__func__,__FILE__);
    return 0;
}

static const struct i2c_device_id lcd16x2_i2c_id[] = {
        { SLAVE_DEVICE_NAME, 0 },
        { }
};
MODULE_DEVICE_TABLE(i2c, lcd16x2_i2c_id);

static struct i2c_driver lcd16x2_i2c_driver = {
	.driver = {
		.name	= SLAVE_DEVICE_NAME,
		.owner	= THIS_MODULE,
	},
	.probe = lcd16x2_i2c_probe,
	.remove = lcd16x2_i2c_remove,
	.id_table = lcd16x2_i2c_id,
};

static struct i2c_board_info lcd16x2_i2c_board_info = {
        I2C_BOARD_INFO(SLAVE_DEVICE_NAME, LCD16X2_SLAVE_ADDR)
    };


static int __init lcd16x2_i2c_client_init(void)
{
	int ret;
    struct device *pdevice;
    
	printk(KERN_INFO"%d:%s:%s\n",__LINE__,__func__,__FILE__);
    
	devno = MKDEV(major, 0);//major nymber is choosen dynamically

    ret = alloc_chrdev_region(&devno, 0, 1,"lcd16x2_i2c_client");
    if(ret < 0)
    {
        printk(KERN_INFO"%s : alloc_chardev_region() failed!\n",__FILE__);
        return -1;
    }

	pclass = class_create(THIS_MODULE, "lcd16x2_i2c_client");
    if(IS_ERR(pclass))
    {
        printk(KERN_INFO"%s : class_create() failed!\n",__FILE__);
        unregister_chrdev_region(devno, 1);
        return -1;
    }

    major = MAJOR(devno);
    printk(KERN_INFO"%s : alloc_chardev_region() : Allocated device number %d / %d!\n",__FILE__, major, MINOR(devno));

    //create device file
    pdevice = device_create(pclass, NULL, devno, NULL,"lcd16x2_i2c_client %d", 0);
    if(IS_ERR(pdevice))
    {
        printk(KERN_INFO"%s : device_create() failed!\n",__FILE__);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO"%s : device_create() Successfull!\n",__FILE__);



//i2c driver
    //get i2c bus(adaptor)
    lcd16x2_i2c_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    if (lcd16x2_i2c_adapter == NULL)
    {
        device_destroy(pclass, devno);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);    
        return -1;   
    }
    printk(KERN_INFO"%s : i2c_get_adapter() Successfull!\n",__FILE__);

    lcd16x2_i2c_client = i2c_new_device(lcd16x2_i2c_adapter,&lcd16x2_i2c_board_info);
    if (lcd16x2_i2c_client == NULL)
    {
        device_destroy(pclass, devno);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);    
        return -1;   
    }
    printk(KERN_INFO"%s : i2c_new_device() Successfull!\n",__FILE__);

    i2c_add_driver(&lcd16x2_i2c_driver);
    printk(KERN_INFO"%s : i2c_add_driver() Successfull!\n",__FILE__);

    i2c_put_adapter(lcd16x2_i2c_adapter);
    printk(KERN_INFO"%s : i2c_put_adapter() Successfull!\n",__FILE__);


    pr_info("Driver ADDED!!!\n");    

    return 0;
}


static void __exit lcd16x2_i2c_client_exit(void)
{
    printk(KERN_INFO"%d:%s:%s\n",__LINE__,__func__,__FILE__);

    i2c_del_driver(&lcd16x2_i2c_driver);
    printk(KERN_INFO"%s : i2c_del_driver() invoked !\n",__FILE__);

    device_destroy(pclass, devno);
    printk(KERN_INFO"%s : device_destroy() destroy dev file !\n",__FILE__);

    
    class_destroy(pclass);
    printk(KERN_INFO"%s : class_destroy() destroy dev class !\n",__FILE__);

    unregister_chrdev_region(devno, 1);
    printk(KERN_INFO"%s : unregister_chardev_reg() released dev num !\n",__FILE__);


}

module_init(lcd16x2_i2c_client_init);
module_exit(lcd16x2_i2c_client_exit);
MODULE_LICENSE("GPL");



/*
||----------------------->>> REFRENCES <<<--------------------------------||
| * kernel.org/doc/Documentation/i2c/writing-clients
| * docs.kernel.org/i2c/index.html
|
|_________________________________________________________________________||
*/