/*
>> UPDATE IN THIS FILE: MENU DRIVEN PROGRAMME TO CHECK EACH
                        FUNCTIONALITY OF LCD DEVICE DRIVER.

    >STATUS : COMPLETE

*/
#include "lcd16x2.h"
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/ioctl.h>

static int fd;
static int choice;
//_______FUNCTION_DECLARATION___________________________________//
int menu_driven (void);
static int fd;
void lcd_delay(int delay);

//______________________________________________________________//
int main (void)
{
    char str[] = "LCD Device Driver" ;
    int ret, delay_msec;
    ioctl_msg_t ioctl_msg;
    char temp[2];
    printf("Welcome to the LCD Device Driver Test Programme\n");

//LCD16X2 FILE OPEN
    fd = open("/dev/lcd16x2 0",O_RDWR);
    if(fd < 0)
    {
        perror("open() failed");
        _exit(1);
    }

    while(1)
    {
        choice = menu_driven();
        switch(choice)
        {
            case 0: //for exit
                ret = close(fd);
                if(ret != 0)
                {
                    perror("closed() failed \n");
                    _exit(1);
                }
                else
                    printf("closed() file call\n");
                break;

            case 1:
                printf("|9. LCD WRITE                \n");
                //printf("Enter a string : ");
                //scanf("%s",str);
                ret = write(fd,str, sizeof(str));
                if(ret < 0)
                {
                    perror("write() failed");
                    _exit(1);
                }
                break;
            
            case 2:
                printf("1. LCD DISPLAY CLEAR                            \n");
                ret = ioctl(fd,IOCTL_LCD_DISP_CLEAR); //LCD_CLEAR flag
                if(ret < 0)
                    perror("ioctl(): failed to clear the screen");
                else
                    printf("ioctl(): LCD Screen is Clear Now\n");

                break;
            
            case 3:
                printf("2. LCD CURSOR BLINK            \n");
                ioctl(fd,IOCTL_LCD_CURSOR_BLINK); //cursor blink off using ioctl
                printf("IOCTL() : Cursor blink CALLED\n");                    
                break;

            case 4:
                printf("3. LCD CURSOR BLINK OFF         \n");
                ioctl(fd,IOCTL_LCD_CURSOR_OFF); //cursor blink off using ioctl
                printf("IOCTL() : Cursor blink OFF CALLED\n");
                break;

            case 5:
                printf("4. LCD DISPLAY SCROLL RIGHT     \n");
                for (int i = 0; i < sizeof(str); i++)
                {
                    lcd_delay(500);
                    ioctl(fd, IOCTL_LCD_DISP_SHIFT_RIGHT);
                    printf("LCD screen shift left \n");
                }
                break;

            case 6:
                printf("5. LCD DISPLAY SCROLL LEFT     \n");
                for (int i = 0; i < sizeof(str); i++)
                {
                    lcd_delay(500);
                    ioctl(fd, IOCTL_LCD_DISP_SHIFT_LEFT);
                    printf("LCD screen shift left \n");
                }
                break;

            case 7:
                printf("6. IOCTL_LCD_SHIFT_CURSOR_POS_LEFT   \n");
                ioctl(fd, IOCTL_LCD_SHIFT_CURSOR_POS_LEFT);
                printf("IOCTL() : IOCTL_LCD_SHIFT_CURSOR_POS_LEFT CALLED\n");

                break;

            case 8:
                printf("7. IOCTL_LCD_SHIFT_CURSOR_POS_RIGHT  \n");
                ioctl(fd, IOCTL_LCD_SHIFT_CURSOR_POS_RIGHT);
                printf("IOCTL() : IOCTL_LCD_SHIFT_CURSOR_POS_RIGHT CALLED\n");

                break;

            case 9:
                printf("6. FORCE CURSOR TO BEGINNING OF 1ST LINE   \n");
                ioctl(fd, IOCTL_LCD_BEGINNING_1ST_LINE);
                printf("IOCTL() : Cursor blink OFF CALLED\n");

                break;

            case 10:
                printf("7. FORCE CURSOR TO BEGINNING OF 2ST LINE   \n");
                ioctl(fd, IOCTL_LCD_BEGINNING_2ST_LINE);
                printf("IOCTL() : Cursor blink OFF CALLED\n");

                break;
            
            case 11:
                printf("|11. LCD RETURN TO HOME              \n");
                ret = ioctl(fd, IOCTL_LCD_RETURN_HOME); //call ioctl lcd return to hime function
                break;

                break;
            case 12:
                printf("12. LCD GIVE DIRECT COMMAND  \n");
                printf("Enter command for LCD16x2 : ");
                scanf("%s",temp);
                printf("%s\n",temp);
                strcpy(ioctl_msg.direct_cmd, temp);
                ret = ioctl(fd,IOCTL_LCD_GIVE_DIRECT_COMMAND, &ioctl_msg);
                if (ret < 0)
                    perror("error : Lcd_given_direct_cmd\n");
                break;
    
            case 13:
                printf("|9. LCD DELAY                \n");
                printf("Enter delay in seconf \n");
                scanf("%d",&delay_msec);
                lcd_delay(delay_msec);
                break;

            default :
                printf("Invalid choice ! \n");
                break;
        }
    }
    return 0;
}

//_______FUNCTION_INITIALIZATION__________________________________//


int menu_driven (void)
{
    
    printf("\n_____________________________________________\n");
    printf("||__________LCD16X2 DEVICE DRIVER__________||\n");
    printf("|0. FOR EXIT                                |\n");
    printf("|1. LCD WRITE                               |\n");
    printf("|2. LCD DISPLAY CLEAR                       |\n");
    printf("|3. LCD CURSOR BLINK                        |\n");
    printf("|4. LCD CURSOR BLINK OFF                    |\n");
    printf("|5. LCD DISPLAY SCROLL RIGHT                |\n");
    printf("|6. LCD DISPLAY SCROLL LEFT                 |\n");
    
    printf("|7. IOCTL_LCD_SHIFT_CURSOR_POS_LEFT         |\n");  
    printf("|8. IOCTL_LCD_SHIFT_CURSOR_POS_RIGHT        |\n"); 
    printf("|9. FORCE CURSOR TO BEGINNING OF 1ST LINE   |\n");
    printf("|10.FORCE CURSOR TO BEGINNING OF 2ST LINE   |\n");

    printf("|11.LCD RETURN HOME                         |\n");

    printf("|12. LCD GIVE DIRECT COMMAND                |\n");

    printf("|___________________________________________|\n");
    printf(">>>Enter your choice :");
    scanf("%d",&choice);
    
    return choice;
}

void lcd_delay(int delay)
{
    for(int i=0; i<delay; i++)       //delay
    {
        ioctl(fd, IOCTL_LCD_DELAY_MSEC);
    }

}



/*
    Comman line argumnet : argument will be display on  lcd16x2 for 1 sec

    how to run this programme?
    1. load the lcd16x2 driver in to kernel module
    2. compile this programme
        >> gcc test.c
    3. run this programme with command line argument
        >> sudo ./a.out <cmd line argument>
    4. watch output on lcd16x2 display 
    5. remove the lcd driver from the kernel module

*/
