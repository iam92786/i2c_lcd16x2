/*
>> PROJECT TITEL : LCD16x2(HD44780) DEVICE DRIVER FOR LINUX(DEBIAN) OS 
>> PLATFORM(HARDWARE) : BEHGALBONE BLACK 
>> DEVLOPED BY : SIDDIQUI ARSHAD HUSSAIN 
>> PG-DESD 2022
>> DESCRIPTION : A KERNEL LEVEL LINUX DEVICE DRIVER TO CONTROL A 16X2 CHARCTER LCD(WITH HD44780 CONTROLLER)
                    WITH 4-BIT MODE. THE LCD IS INTERFACED WITH A MICRO CONTROLLER USING GPIO PINS.

>> STATUS/UPDATE : COMPLETED / DONE
*/

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>
#include<linux/gpio.h> //for gpiolib function
#include<linux/delay.h> //for mdelay()
#include<linux/uaccess.h> //for copy_to_user () function in used in write()
#include<linux/slab.h> //kmalloc()
#include"lcd16x2.h"

//Buffer
#define MAXLEN 80 //buffer size

//data pin of lcd       BBB pin number
#define GPIO_LCD_DB7    67
#define GPIO_LCD_DB6    68
#define GPIO_LCD_DB5    44
#define GPIO_LCD_DB4    26

//control pin of the lcd   BBB pin number
#define GPIO_LCD_ENABLE    66
#define GPIO_LCD_RESET     69
//#define GPIO_LCD_RDWR    68

#define PIN_RESET      0
#define PIN_SET        1

//global varible
static int major = 250;
static dev_t devno;
static struct class *pclass;
static struct cdev lcd16x2_cdev;

//FILE OPERATION FUNCTION DECLARATION
static int lcd_open (struct inode *pinode, struct file *pfile);
static ssize_t lcd_read (struct file *pinode, char __user *pbuf, size_t length, loff_t *pfpos);
static ssize_t lcd_write (struct file *pinode, const char __user *pbuf, size_t length, loff_t *pfpos);
static int lcd_release (struct inode *pinode, struct file *);
static long lcd_ioctl(struct file *pfile, unsigned int cmd, unsigned long param);

static struct file_operations lcd16x2_fops = {
    .owner = THIS_MODULE,
    .open = lcd_open,
    .release = lcd_release,
    .write = lcd_write,
    .read = lcd_read,
    .unlocked_ioctl = lcd_ioctl
};

static int gpio_lcd_init(int GPIO_PIN_NUM, char GPIO_LABEL[16]);
static int lcd16x2_init_4bit(void);
static int lcd16x2_write_4bit(uint8_t); //this function doesnot do anything with control PIN of LCD
static int lcd16x2_write_4bit_cmd(uint8_t);
static int lcd16x2_write_data(uint8_t);
static int lcd16x2_enable(void);


//Additional Functionality of lcd16x2
static void lcd16x2_screen_clear(void); //0x01
static void lcd16x2_delay(void);
static void lcd16x2_cursor_blink(void);//0x0f display on cursor blink
static void lcd16x2_cursor_blink_off(void);//0x0c display on cursor off
static void lcd16x2_shift_right(void);//0x1c display shift right
static void lcd16x2_shift_left(void);//0x18 didplay shift left 

static void lcd16x2_return_home(void); //0x02 clear display screen
static void lcd16x2_beginning_first_line(void); //0x80 beginning ist line 
static void lcd16x2_beginning_second_line(void);

static void lcd16x2_shift_cursor_pos_right(void);
static void lcd16x2_shift_cursor_pos_left(void);




//_______________________________________________________________________________________//
static int lcd16x2_init_4bit(void)
{
    printk(KERN_INFO"%s lcd16x2_init_4bit() called !\n",THIS_MODULE->name);
    mdelay(50); //20msec delay after turn on of LCD
    lcd16x2_write_4bit_cmd(0x3);
    
    mdelay(5); //5msec delay
    lcd16x2_write_4bit_cmd(0x3);

    mdelay(5); //5msec delay
    lcd16x2_write_4bit_cmd(0x3);

    mdelay(5); //5msec delay
    lcd16x2_write_4bit_cmd(0x2);


//for 4-bit mode = 0x28 is given to the LCD
    mdelay(5); //5msec delay
    lcd16x2_write_4bit_cmd(0x2);

    mdelay(1); //5msec delay
    lcd16x2_write_4bit_cmd(0x8);


//for disp ON, and cuesor blink = 0x0f given to the LCD
    mdelay(5); //5msec delay
    lcd16x2_write_4bit_cmd(0x0);

    mdelay(1); //5msec delay
    lcd16x2_write_4bit_cmd(0xf);

// for disp clear = 0x01
    mdelay(5);
    lcd16x2_write_4bit_cmd(0x0);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0x1);
    mdelay(5);

    return 0;
}

static int lcd16x2_write_4bit_cmd(uint8_t cmd)
{
    printk(KERN_INFO"%s lcd16x2_write_4bit_cmd() called !\n",THIS_MODULE->name);

    gpio_set_value(GPIO_LCD_RESET, PIN_RESET);  //rs = 0 for command
    lcd16x2_write_4bit(cmd);                    //lcd write

    return 0;
}
static int lcd16x2_write_data(uint8_t data)
{
    uint8_t lsb_nib , msb_nib;
    printk(KERN_INFO"%s lcd16x2_write_4bit_data() called !\n",THIS_MODULE->name);
    gpio_set_value(GPIO_LCD_RESET,PIN_SET); //rs = 1 for data
    
    lsb_nib = data&0xf;
    msb_nib = (data >> 4)&0xf;

    lcd16x2_write_4bit(msb_nib);               //lcd write

    lcd16x2_write_4bit(lsb_nib);               //lcd write
    return 0;
}

static int lcd16x2_write_4bit(uint8_t nibble) //this function doesnot do anything with control PIN of LCD
{    
    //take 4-bit out of 8-bit
    //rintk(KERN_INFO"%s lcd16x2_write_4bit() called !\n",THIS_MODULE->name);
    //nibble &= 0xf;
    gpio_set_value(GPIO_LCD_DB4, (nibble & 0x01)); // LSB 0-bit
    gpio_set_value(GPIO_LCD_DB5, ((nibble >> 1)& 0x01));
    gpio_set_value(GPIO_LCD_DB6, ((nibble >> 2)& 0x01));
    gpio_set_value(GPIO_LCD_DB7, ((nibble >> 3)& 0x01)); //MSB 3rd-bit

    lcd16x2_enable();
    return 0;
}

static int lcd16x2_enable(void)
{
    //printk(KERN_INFO"%s lcd16x2_enable() called !\n",THIS_MODULE->name);
    gpio_set_value(GPIO_LCD_ENABLE, PIN_SET); //enable pin set = 1
    mdelay(1);          //delay of 1msec
    gpio_set_value(GPIO_LCD_ENABLE, PIN_RESET); //enable pin = 0
    mdelay(1);
    return 0;
}



static int __init lcd16x2_device_init(void)
{
    int ret, gpio_ret, lcd_ret;
    
    struct device *pdevice;
    printk(KERN_INFO"%s lcd16x2_init called() !\n",THIS_MODULE->name);

//allocate device number dynamically
    devno = MKDEV(major, 0);//major nymber is choosen dynamically

    ret = alloc_chrdev_region(&devno, 0, 1,"lcd16x2");
    if(ret < 0)
    {
        printk(KERN_INFO"%s alloc_chardev_region() failed!\n",THIS_MODULE->name);
        return -1;
    }
    major = MAJOR(devno);
    printk(KERN_INFO"%s alloc_chardev_region() allocated dev number %d / %d!\n",
                THIS_MODULE->name, major, MINOR(devno));

//create device class
    pclass = class_create(THIS_MODULE, "lcd16x2_class");
    if(IS_ERR(pclass))
    {
        printk(KERN_INFO"%s class_create() failed!\n",THIS_MODULE->name);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO"%s class_create() Successfull!\n",THIS_MODULE->name);

//create device file
    pdevice = device_create(pclass, NULL, devno, NULL,"lcd16x2 %d", 0);
    if(IS_ERR(pdevice))
    {
        printk(KERN_INFO"%s device_create() failed!\n",THIS_MODULE->name);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO"%s device_create() Successfull!\n",THIS_MODULE->name);

//initialize cdev var and add in to kernel database
    cdev_init(&lcd16x2_cdev, &lcd16x2_fops);
    ret = cdev_add(&lcd16x2_cdev, devno, 1);
    if(ret < 0)
    {
        printk(KERN_INFO"%s cdev_add() failed!\n",THIS_MODULE->name);
        device_destroy(pclass, devno);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO"%s cdev_add() Successfull, char dev added in to the kernel!\n",THIS_MODULE->name);


//INITIALIZATION OF GPIO PIN USED FOR LCD DATA PIN
    gpio_ret = gpio_lcd_init(GPIO_LCD_DB7, "DB7");
    if(gpio_ret != 0)
    {
        printk(KERN_INFO"%s: gpio %d initialization failed .\n",THIS_MODULE->name, GPIO_LCD_DB7);
    }
    printk(KERN_INFO"%s: gpio %d initialization is Successfull. \n",THIS_MODULE->name, GPIO_LCD_DB7);
    
    gpio_ret = gpio_lcd_init(GPIO_LCD_DB6, "DB6");
    if(gpio_ret != 0)
    {
        printk(KERN_INFO"%s: gpio %d initialization failed .\n",THIS_MODULE->name, GPIO_LCD_DB7);
    }
    printk(KERN_INFO"%s: gpio %d initialization is Successfull. \n",THIS_MODULE->name, GPIO_LCD_DB7);
    
    gpio_ret = gpio_lcd_init(GPIO_LCD_DB5, "DB5");
    if(gpio_ret != 0)
    {
        printk(KERN_INFO"%s: gpio %d initialization failed .\n",THIS_MODULE->name, GPIO_LCD_DB7);
    }
    printk(KERN_INFO"%s: gpio %d initialization is Successfull. \n",THIS_MODULE->name, GPIO_LCD_DB7);
    
    gpio_ret = gpio_lcd_init(GPIO_LCD_DB4, "DB4");
    if(gpio_ret != 0)
    {
        printk(KERN_INFO"%s: gpio %d initialization failed .\n",THIS_MODULE->name, GPIO_LCD_DB7);
    }
    printk(KERN_INFO"%s: gpio %d initialization is Successfull. \n",THIS_MODULE->name, GPIO_LCD_DB7);
    

//INITIALIZATION OF GPIO PIN USED FOR LCD CONTROL PIN (ENABLE, RESET)
    gpio_ret = gpio_lcd_init(GPIO_LCD_ENABLE, "ENABLE");
    if(gpio_ret != 0)
    {
        printk(KERN_INFO"%s: gpio %d initialization failed .\n",THIS_MODULE->name, GPIO_LCD_ENABLE);
    }
    printk(KERN_INFO"%s: gpio %d initialization is Successfull. \n",THIS_MODULE->name, GPIO_LCD_ENABLE);
    
    gpio_ret = gpio_lcd_init(GPIO_LCD_RESET, "RESET");
    if(gpio_ret != 0)
    {
        printk(KERN_INFO"%s: gpio %d initialization failed .\n",THIS_MODULE->name, GPIO_LCD_RESET);
    }
    printk(KERN_INFO"%s: gpio %d initialization is Successfull. \n",THIS_MODULE->name, GPIO_LCD_RESET);


//---------------------LCD initialization--------------------------------------//
    lcd_ret = lcd16x2_init_4bit();
    if(lcd_ret != 0)
    {
        printk(KERN_INFO"%s lcd16x2_init() failed !\n",THIS_MODULE->name);
    }
    printk(KERN_INFO"%s lcd16x2_init() Successfull !\n",THIS_MODULE->name);

//--------------------LCD display character data send --------------------------//

    //instruction will come from user space programme
    
    return 0;
}

static void __exit lcd16x2_device_exit (void)
{
    printk(KERN_INFO"%s lcd16x2_exit() called !\n",THIS_MODULE->name);
//released gpio pin
    gpio_free(GPIO_LCD_DB7);
    printk(KERN_INFO"%s gpio %d is relesde, \n",THIS_MODULE->name, GPIO_LCD_DB7);

    gpio_free(GPIO_LCD_DB6);
    printk(KERN_INFO"%s gpio %d is relesde, \n",THIS_MODULE->name, GPIO_LCD_DB6);

    gpio_free(GPIO_LCD_DB5);
    printk(KERN_INFO"%s gpio %d is relesde, \n",THIS_MODULE->name, GPIO_LCD_DB5);

    gpio_free(GPIO_LCD_DB4);
    printk(KERN_INFO"%s gpio %d is relesde, \n",THIS_MODULE->name, GPIO_LCD_DB4);

    gpio_free(GPIO_LCD_ENABLE);
    printk(KERN_INFO"%s gpio %d is relesde, \n",THIS_MODULE->name, GPIO_LCD_ENABLE);

    gpio_free(GPIO_LCD_RESET);
    printk(KERN_INFO"%s gpio %d is relesde, \n",THIS_MODULE->name, GPIO_LCD_RESET);


    cdev_del(&lcd16x2_cdev);
    printk(KERN_INFO"%s cdev_del() del char dev from kernel !\n",THIS_MODULE->name);

    device_destroy(pclass, devno);
    printk(KERN_INFO"%s device_destroy() destroy dev file !\n",THIS_MODULE->name);
    
    class_destroy(pclass);
    printk(KERN_INFO"%s class_destroy() destroy dev class !\n",THIS_MODULE->name);
    
    unregister_chrdev_region(devno, 1);
    printk(KERN_INFO"%s unregister_chardev_reg() released dev num !\n",THIS_MODULE->name);

}

int gpio_lcd_init(int GPIO_PIN_NUM, char GPIO_LABEL[8])
{
    printk(KERN_INFO"%s gpio_lcd_init() called !\n",THIS_MODULE->name);
    if(gpio_is_valid(GPIO_PIN_NUM) == false)
    {
        printk(KERN_INFO"%s gpio_is_valid() failed -- invalid gpio pin !\n",THIS_MODULE->name);
        cdev_del(&lcd16x2_cdev);
        device_destroy(pclass, devno);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 0);
        return -1;    
    }
    printk(KERN_INFO"%s : gpio %d is valid !\n",THIS_MODULE->name,GPIO_PIN_NUM);

    if(gpio_request(GPIO_PIN_NUM,GPIO_LABEL) < 0)
    {
        printk(KERN_INFO"%s gpio_request() failed --gpio busy !\n",THIS_MODULE->name);
        cdev_del(&lcd16x2_cdev);
        device_destroy(pclass, devno);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 0);
        return -1; 
    }
    printk(KERN_INFO"%s : gpio %d is valid !\n",THIS_MODULE->name,GPIO_PIN_NUM);

    if(gpio_direction_output(GPIO_PIN_NUM, 1) < 0)
    {
        printk(KERN_INFO"%s gpio_direction_output() failed --gpio busy !\n",THIS_MODULE->name);
        gpio_free(GPIO_PIN_NUM);
        cdev_del(&lcd16x2_cdev);
        device_destroy(pclass, devno);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 0);
        return -1; 
    }
    printk(KERN_INFO"%s : gpio %d direction is set to output!\n",THIS_MODULE->name,GPIO_PIN_NUM);

    return 0;
}


int lcd_open (struct inode *pinode, struct file *pfile)
{
    printk("%s lcd_open() called !\n",THIS_MODULE->name);
    return 0;
}

ssize_t lcd_read (struct file *pinode, char __user *pbuf, size_t length, loff_t *pfpos)
{
    printk("%s lcd_read() called !\n",THIS_MODULE->name);
    return 0;

}

ssize_t lcd_write (struct file *pfile, const char __user *pubuf, size_t length, loff_t *pfpos)
{
    static char buffer[MAXLEN] = "NULL";

    int bytes_to_write, bytes_not_copied, nbytes,  i;

    //02 how many bytes to be copied into device buffer
    bytes_to_write = MAXLEN >= length ? MAXLEN : length;

    //03 if num of bytes to write is zero, return error(on space available)
    if(bytes_to_write == length)
    {
        printk("%s lcd16x2_write : device buffer is 80 char only !\n",THIS_MODULE->name);
        return -ENOSPC; //error no space
    }


    //04 copy the bytes from user buffer to device buffer
    bytes_not_copied = copy_from_user(buffer, pubuf, bytes_to_write);

    //05 calculate number of bytes succefully copied
    nbytes = bytes_to_write - bytes_not_copied;

    //07 load buffer data in to the lcd16x2 display using "lcd16x2_write_data"
    for(i=0; i < (length-1); i++)
    {
        lcd16x2_write_data(buffer[i]); //41H = A
    }

    printk(KERN_INFO"%s lcd16x2_ driver write()\n",THIS_MODULE->name);
    //returns numberr of bytes Successfully copied
    return length;
}

int lcd_release (struct inode *pinode, struct file *pfile)
{
    printk("%s lcd16x2_exit called() !\n",THIS_MODULE->name);
    return 0;

}

long lcd_ioctl(struct file *pfile, unsigned int cmd, unsigned long param)
{

    char lsb_nib , msb_nib;
    ioctl_msg_t ioctl_msg;

    printk("%s lcd_ioctl called() !\n",THIS_MODULE->name);

    switch(cmd){
        case IOCTL_LCD_DISP_CLEAR://01
        lcd16x2_screen_clear();
        printk(KERN_INFO"%s lcd_ioctl() - IOCTL_LCD_DISP_CLEAR\n",THIS_MODULE->name);
            break;

        case IOCTL_LCD_DELAY_MSEC://02
        lcd16x2_delay();
        printk(KERN_INFO"%s lcd_ioctl() - IOCTL_LCD_DELAY_MSEC \n",THIS_MODULE->name);
            break;
        
        case IOCTL_LCD_CURSOR_BLINK://03
        lcd16x2_cursor_blink();
        printk(KERN_INFO"%s lcd_ioctl() -IOCTL_LCD_CURSOR_BLINK \n",THIS_MODULE->name);
            break;
        
        case IOCTL_LCD_CURSOR_OFF://04
        lcd16x2_cursor_blink_off();
        printk(KERN_INFO"%s lcd_ioctl() -IOCTL_LCD_CURSOR_OFF \n",THIS_MODULE->name);
            break;
        
        case IOCTL_LCD_DISP_SHIFT_RIGHT://05
        lcd16x2_shift_right();
        printk(KERN_INFO"%s lcd_ioctl() - IOCTL_LCD_DISP_SHIFT_RIGHT\n",THIS_MODULE->name);
            break;
        
        case IOCTL_LCD_DISP_SHIFT_LEFT://06
        lcd16x2_shift_left();
        printk(KERN_INFO"%s lcd_ioctl() -IOCTL_LCD_DISP_SHIFT_LEFT\n",THIS_MODULE->name);
            break;

        case IOCTL_LCD_SHIFT_CURSOR_POS_LEFT://07
        lcd16x2_shift_cursor_pos_left();
        printk(KERN_INFO"%s lcd_ioctl() -IOCTL_LCD_SHIFT_CURSOR_POS_LEFT\n",THIS_MODULE->name);
            break;

        case IOCTL_LCD_SHIFT_CURSOR_POS_RIGHT://08
        lcd16x2_shift_cursor_pos_right();
        printk(KERN_INFO"%s lcd_ioctl() -IOCTL_LCD_SHIFT_CURSOR_POS_RIGHT\n",THIS_MODULE->name);
            break;


        case IOCTL_LCD_BEGINNING_1ST_LINE://09
        lcd16x2_beginning_first_line();
        printk(KERN_INFO"%s lcd_ioctl() -IOCTL_LCD_BEGINNING_1ST_LINE\n",THIS_MODULE->name);
            break;

        case IOCTL_LCD_BEGINNING_2ST_LINE://10
        lcd16x2_beginning_second_line();
        printk(KERN_INFO"%s lcd_ioctl() -IOCTL_LCD_BEGINNING_2ST_LINE\n",THIS_MODULE->name);
            break;


        case IOCTL_LCD_RETURN_HOME://11
        lcd16x2_return_home();
        printk(KERN_INFO"%s lcd_ioctl() -IOCTL_LCD_RETURN_HOME\n",THIS_MODULE->name);
            break;

        case IOCTL_LCD_GIVE_DIRECT_COMMAND://12
        //give direct command to the lcd

            if(copy_from_user(&ioctl_msg, (const void *)param, sizeof(ioctl_msg)))
            {
                printk(KERN_INFO"%s IOCTL() : IOCTL_LCD_GIVE_DIRECT_COMMAND : failed to copy from user space pro\n",THIS_MODULE->name);
                return -EFAULT;
            }

            msb_nib = ioctl_msg.direct_cmd[0]; //temp1 = cmd
            lsb_nib = ioctl_msg.direct_cmd[1]; //temp1 = cmd

            //lcd write

            lcd16x2_write_4bit_cmd(msb_nib);           
            mdelay(1);
            lcd16x2_write_4bit_cmd(lsb_nib);  
            mdelay(1); 
            printk(KERN_INFO"%s lcd_ioctl() -IOCTL_LCD_GIVE_DIRECT_COMMAND\n",THIS_MODULE->name);
            break;

        default:
            printk(KERN_INFO"%s lcd_ioctl() - invalid operation \n",THIS_MODULE->name);
            return -EINVAL;
    }



    return 0;
}



//______________________Additional Functionality of lcd16x2, used in IOCTL___________________________//



static void lcd16x2_shift_cursor_pos_right(void)//0x14
{
    lcd16x2_write_4bit_cmd(0x1);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0x4);
    mdelay(1);
    printk(KERN_INFO"%s lcd16x2_shift_cursor_pos_right() \n",THIS_MODULE->name);
}

static void lcd16x2_shift_cursor_pos_left(void)//0x10
{
    lcd16x2_write_4bit_cmd(0x1);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0x0);
    mdelay(1);
    printk(KERN_INFO"%s lcd16x2_shift_cursor_pos_left() !\n",THIS_MODULE->name);
}

static void lcd16x2_beginning_first_line(void) //0x80 beginning ist line 
{
    lcd16x2_write_4bit_cmd(0x8);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0x0);
    mdelay(1);
    printk(KERN_INFO"%s lcd16x2_beginning_first_line() !\n",THIS_MODULE->name);
}

static void lcd16x2_beginning_second_line(void) //0xco beginning 2nd line
{
    lcd16x2_write_4bit_cmd(0xc);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0x0);
    mdelay(1);
    printk(KERN_INFO"%s lcd16x2_beginning_second_line() !\n",THIS_MODULE->name);
}

static void lcd16x2_return_home(void) //0x02 clear display screen
{
    lcd16x2_write_4bit_cmd(0x0);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0x2);
    mdelay(1);
    printk(KERN_INFO"%s lcd16x2_return_home() !\n",THIS_MODULE->name);
}

static void lcd16x2_screen_clear(void) //0x01 clear display screen
{
    lcd16x2_write_4bit_cmd(0x0);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0x1);
    mdelay(1);
    printk(KERN_INFO"%s display_screen_clear() !\n",THIS_MODULE->name);
}

static void lcd16x2_delay(void) //delay in msec
{
    mdelay(1);
    printk(KERN_INFO"%s lcd16x2_delay() !\n",THIS_MODULE->name);
}

static void lcd16x2_cursor_blink(void)//0x0f display on cursor blink
{
    lcd16x2_write_4bit_cmd(0x0);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0xf);
    mdelay(1);
    printk(KERN_INFO"%s lcd16x2_cursor_on() !\n",THIS_MODULE->name);
}
static void lcd16x2_cursor_blink_off(void)//0x0c display on cursor off
{
    lcd16x2_write_4bit_cmd(0x0);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0xc);
    mdelay(1);
    printk(KERN_INFO"%s lcd16x2_cursor_blink_off() !\n",THIS_MODULE->name);
}
static void lcd16x2_shift_right(void)//0x1c display shift right
{
    lcd16x2_write_4bit_cmd(0x1);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0xc);
    mdelay(1);
    printk(KERN_INFO"%s lcd16x2_shift_right() !\n",THIS_MODULE->name);
}
static void lcd16x2_shift_left(void)//0x18 didplay shift left
{
    lcd16x2_write_4bit_cmd(0x1);
    mdelay(1);
    lcd16x2_write_4bit_cmd(0x8);
    mdelay(1);
    printk(KERN_INFO"%s lcd16x2_shift_left() !\n",THIS_MODULE->name);
} 

module_init(lcd16x2_device_init);
module_exit(lcd16x2_device_exit);

MODULE_LICENSE("GPL");

MODULE_DESCRIPTION("HD44780(lcd16x2) Device Driver For Beagle Bone Black(BBB)");

MODULE_AUTHOR("Arshad <ars92786@gmail.com>");
