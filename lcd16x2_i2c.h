



#define ERROR -1
#define I2C_BUS_AVAILABLE 2
#define SLAVE_DEVICE_NAME "lcd16x2_i2c_client"
#define LCD16X2_SLAVE_ADDR  0x27

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



