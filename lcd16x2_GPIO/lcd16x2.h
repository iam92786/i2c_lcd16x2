#ifndef __lcd16x2_h
#define __lcd16x2_h

#include <linux/ioctl.h>

typedef struct ioct_struct{
    char direct_cmd[2];
}ioctl_msg_t;

#define IOCTL_LCD_DISP_CLEAR                _IO('x',1)
#define IOCTL_LCD_DELAY_MSEC                _IO('x',2)         //identifier for ioctl request
#define IOCTL_LCD_CURSOR_BLINK              _IO('x',3)
#define IOCTL_LCD_CURSOR_OFF                _IO('x',4)
#define IOCTL_LCD_DISP_SHIFT_RIGHT          _IO('x',5)
#define IOCTL_LCD_DISP_SHIFT_LEFT           _IO('x',6)
#define IOCTL_LCD_SHIFT_CURSOR_POS_LEFT     _IO('X',7)
#define IOCTL_LCD_SHIFT_CURSOR_POS_RIGHT    _IO('X',8)
#define IOCTL_LCD_BEGINNING_1ST_LINE        _IO('X',9)
#define IOCTL_LCD_BEGINNING_2ST_LINE        _IO('X',10)

//#define IOCTL_LCD_GENERALIZE_POSITION       _IOW('X',11,pos_t)
#define IOCTL_LCD_GIVE_DIRECT_COMMAND       _IOW('X',12,ioctl_msg_t)

#define IOCTL_LCD_RETURN_HOME               _IO('X',13)


#endif