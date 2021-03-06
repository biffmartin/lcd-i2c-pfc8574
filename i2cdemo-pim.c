#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/ioctl.h>
#include "linux/i2c-dev.h"

/* ---------------------------------------------------------
 * Author: Mike Martin               Date May 16, 2021
 * A Cowboy Programmer
 *
 * i2cdemo-pim.c    
 * I2C LCD Display Module LCD2004 20x4 module with 5x8 chars
 * with PFC8574 controller to I2C,  Converts I2C to 8 bit
 * LCD control.  
 * Raspberry PI connections: SCA, SCL, 5V DC and GND
 * How to Compile:
 * gcc -g i2cdemo-pim.c -o i2cdemo-pim.x
 * How to Run:
 * sudo ./i2cdemo-pim.x
 * This is the main demo program code for the vishay 20x4 lcd
 * Demonstrates how to use the C stdio library to operate lcd
 * -------------------------------------------------------- */

#define _MODE_REGISTER 0x00
#define _PICTURE_MODE 0x00
#define _COLOR_OFFSET 0x24

//# LCD Address
#define ADDRESS  0x27

//# commands
#define LCD_CLEARDISPLAY  0x01
#define LCD_RETURNHOME  0x02
#define LCD_ENTRYMODESET  0x04
#define LCD_DISPLAYCONTROL  0x08
#define LCD_CURSORSHIFT  0x10
#define LCD_FUNCTIONSET  0x20
#define LCD_SETCGRAMADDR  0x40
#define LCD_SETDDRAMADDR  0x80

//# flags for display entry mode
#define LCD_ENTRYRIGHT  0x00
#define LCD_ENTRYLEFT  0x02
#define LCD_ENTRYSHIFTINCREMENT  0x01
#define LCD_ENTRYSHIFTDECREMENT  0x00

//# flags for display on/off control
#define LCD_DISPLAYON  0x04
#define LCD_DISPLAYOFF  0x00
#define LCD_CURSORON  0x02
#define LCD_CURSOROFF  0x00
#define LCD_BLINKON  0x01
#define LCD_BLINKOFF  0x00

//# flags for display/cursor shift
#define LCD_DISPLAYMOVE  0x08
#define LCD_CURSORMOVE  0x00
#define LCD_MOVERIGHT  0x04
#define LCD_MOVELEFT  0x00

//# flags for function set
#define LCD_8BITMODE  0x10
#define LCD_4BITMODE  0x00
#define LCD_2LINE  0x08
#define LCD_1LINE  0x00
#define LCD_5x10DOTS  0x04
#define LCD_5x8DOTS  0x00

//# flags for backlight control
#define LCD_BACKLIGHT  0x08
#define LCD_NOBACKLIGHT  0x00

// enable bit
#define En 0b00000100 
// read/write bit
#define Rw 0b00000010 
// register select
#define Rs 0b00000001 

/* ---------------------------------------------------------
 * forward declarations
 * -------------------------------------------------------- */
int lcd_init( char );
int lcd_clear( int );
int lcd_write_four_bits(int, char);
int lcd_write_char(int, char, char);
int lcd_write( int, char );
int lcd_backlight( int, int );
int lcd_write_string( int, char *, int );
int lcd_read_byte_data( int, char *, int);
int lcd_load_custom_chars( int, int, char **);
int lcd_display_string_pos(int, char *, int, int);

/* let's define a custom icon, consisting of 6 individual characters
 3 chars in the first row and 3 chars in the second row */
char fontdata1[7][8] = {
        /* Char 0 - Upper-left   */
        { 0x00, 0x00, 0x03, 0x04, 0x08, 0x19, 0x11, 0x10 },
        /* Char 1 - Upper-middle   */
        { 0x00, 0x1F, 0x00, 0x00, 0x00, 0x11, 0x11, 0x00 },
        /* Char 2 - Upper-right   */
        { 0x00, 0x00, 0x18, 0x04, 0x02, 0x13, 0x11, 0x01 },
        /* Char 3 - Lower-left   */
        { 0x12, 0x13, 0x1b, 0x09, 0x04, 0x03, 0x00, 0x00 },
        /* Char 4 - Lower-middle   */
        { 0x00, 0x11, 0x1f, 0x1f, 0x0e, 0x00, 0x1F, 0x00 },
        /* Char 5 - Lower-right   */
        { 0x09, 0x19, 0x1b, 0x12, 0x04, 0x18, 0x00, 0x00 },
        /* Char 6 - my test   */
        { 0x1f, 0x0, 0x4, 0xe, 0x0, 0x1f, 0x1f, 0x1f },
};

/* ---------------------------------------------------------
 * test the functions
 * -------------------------------------------------------- */
int main(int argc, char *argv[])
{
    int ix,fd;
    struct timespec _500ms;
    struct timespec _2sec;
    time_t rawtime;
    struct tm *info;
    char timestr[80];
    char buf[80];
    char block[3] = { 0x01, 0x02, 0x0 };
    char pos;
    _500ms.tv_sec = 0;
    _500ms.tv_nsec = 5000000L;
    _2sec.tv_sec = 2;
    _2sec.tv_nsec = 0L;

    fd = lcd_init(0x27);
    lcd_clear(fd);

    /* ---------------------------------------------
     * Demonstrate the Custom Font Feature
     * ------------------------------------------- */
    block[0] = 0x03;
    block[1] = 0x04;
    lcd_load_custom_chars(fd, 7, (char **)fontdata1);
    lcd_display_string_pos(fd, block, 1, 0);
    lcd_display_string_pos(fd, block, 2, 0);
    lcd_display_string_pos(fd, block, 3, 0);
    lcd_display_string_pos(fd, block, 4, 0);
    nanosleep(&_2sec, NULL);
    lcd_clear(fd);

    /* ---------------------------------------------
     * Demonstrate the Single Char Write Feature
     * ------------------------------------------- */
    lcd_write_char(fd, 0x80, 0);   // 0x80 - Line 1
    lcd_write_char(fd, 'A',  1);
    lcd_write_char(fd, 0,   1);
    lcd_write_char(fd, 1,   1);
    lcd_write_char(fd, 2,   1);
    lcd_write_char(fd, 0xC0, 0);   // 0xc0 - Line 2
    lcd_write_char(fd, 'A',  1);
    lcd_write_char(fd,  3,  1);
    lcd_write_char(fd,  4,  1);
    lcd_write_char(fd,  5,  1);
    lcd_write_char(fd, 'C', 1);
    lcd_write_char(fd, 'A', 1);
    lcd_write_char(fd, 'T', 1);
    lcd_write_char(fd, 0x94, 0);   // 0x94 - Line 3
    lcd_write_char(fd, 'A', 1);
    lcd_write_char(fd,  6,  1);
    lcd_write_char(fd,  7,  1);
    lcd_write_char(fd,  8,  1);
    lcd_write_char(fd, 'C', 1);
    lcd_write_char(fd, 'C', 1);
    lcd_write_char(fd, 0xD4, 0);   // 0xD4 - Line 4
    lcd_write_char(fd, 'D', 1);
    lcd_write_char(fd, 'A', 1);
    lcd_write_char(fd, 'T', 1);
    lcd_write_char(fd, 254, 1);
    lcd_write_char(fd, '.', 1);
    lcd_write_char(fd, 'H', 1);
    lcd_write_char(fd, 'E', 1);
    lcd_write_char(fd, 'L', 1);
    lcd_write_char(fd, 'L', 1);
    lcd_write_char(fd, '0', 1);
    lcd_write_char(fd, '?', 1);
    lcd_write_char(fd, '*', 1);
    nanosleep(&_2sec, NULL);


    /* ---------------------------------------------
     * Demonstrate Printing Strings on the LCD
     * ------------------------------------------- */
    lcd_clear(fd);
    lcd_write_string(fd, "The quick brown fox jumps over the lazy dog? ABCDEFGHIJKLMNOPQRSTXYZ", 1);
    nanosleep(&_2sec, NULL);
    // lcd_read_byte_data(fd, buf, 64);

    /* ---------------------------------------------
     * Demonstrate Printing Strings on the LCD
     * ------------------------------------------- */
    lcd_clear(fd);
    for (ix=0; ix<10; ix++) {
       time( &rawtime );
       info = localtime( &rawtime );
       strftime(timestr, 80, "[**Date and Time:**]%A %x     %I:%M:%S %p", info);

       lcd_write_string(fd, timestr, 1);
       nanosleep(&_2sec, NULL);
    }

    /* ---------------------------------------------
     * Shutdown and clear the LCD
     * ------------------------------------------- */
    lcd_clear(fd);
    lcd_write(fd, LCD_DISPLAYCONTROL | LCD_DISPLAYOFF);
    lcd_backlight(fd, 0);

    return 0;

    exit(EXIT_SUCCESS);
}

/* --------------------------------------------------------------
 * lcd_init( int deviceID )
 * arguments: deviceID    the device ID of your PFC8574
 * use i2cdetect -y 1 to see whats there. the A0, A1, and A2
 * pins on your PFC will determine the ID.
 * return value: fd    file handle to the special i2c device
 * ------------------------------------------------------------- */
int lcd_init( char deviceID )
{
    int ix,fd;
    struct timespec _500ms;
    struct timespec _2sec;
    char buf[17];
    _500ms.tv_sec = 0;
    _500ms.tv_nsec = 5000000L;
    _2sec.tv_sec = 2;
    _2sec.tv_nsec = 0L;
    fd = open("/dev/i2c-1", O_RDWR);
    if(fd < 0) {
        fprintf(stderr, "Error opening device\n");
        exit(EXIT_FAILURE);
    }
    if(ioctl(fd, I2C_SLAVE, 0x27) < 0) {
        fprintf(stderr, "Error setting slave address\n");
        close(fd);
        exit(EXIT_FAILURE);
    }

    lcd_write(fd, 0x03);
    lcd_write(fd, 0x03);
    lcd_write(fd, 0x03);
    lcd_write(fd, 0x02);
 
    lcd_write(fd, LCD_FUNCTIONSET | LCD_2LINE | LCD_5x8DOTS | LCD_4BITMODE );
    lcd_write(fd, LCD_DISPLAYCONTROL | LCD_DISPLAYON);
    lcd_write(fd, LCD_CLEARDISPLAY);
    lcd_write(fd, LCD_ENTRYMODESET | LCD_ENTRYLEFT);
    nanosleep(&_500ms, NULL);
    lcd_clear(fd);
    return fd;
}


/* ---------------------------------------------------------
 * -------------------------------------------------------- */
int lcd_clear( int fd )
{
    char data;
    lcd_write_char(fd, LCD_CLEARDISPLAY, 0);
    lcd_write_char(fd, LCD_RETURNHOME, 0);
}


/* ---------------------------------------------------------
 * -------------------------------------------------------- */
int lcd_write( int fd, char buf ) 
{
    if(write(fd, &buf, 1) != 1) {
        fprintf(stderr, "Error writing (1)\n");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Write success!\n");
    return(1);
}

/* ---------------------------------------------------------
 * -------------------------------------------------------- */
int lcd_write_char(int fd, char charval, char mode)
{
    lcd_write_four_bits(fd, mode | (charval & 0xf0));
    lcd_write_four_bits(fd, mode | ((charval << 4) & 0xf0));
}

/* ---------------------------------------------------------
 * -------------------------------------------------------- */
int lcd_write_four_bits(int fd, char buf) 
{
    struct timespec _500ms;
    char data;
    char mode=0x04;
    _500ms.tv_sec = 0;
    _500ms.tv_nsec = 2000000L;
    data = buf | LCD_BACKLIGHT;
    write(fd, &data, 1);
    nanosleep(&_500ms, NULL);
    data = buf | En | LCD_BACKLIGHT;
    write(fd, &data, 1);
    nanosleep(&_500ms, NULL);
    data = (buf & ~En) | LCD_BACKLIGHT;
    write(fd, &data, 1);
    return(1);
}

/* ---------------------------------------------------------
 * lcd_backlight:
 * turn on/off backlight
 * -------------------------------------------------------- */
int lcd_backlight( int fd, int istate)
{
	if (istate == 0){
            lcd_write(fd, LCD_NOBACKLIGHT);
	} else {
            lcd_write(fd, LCD_BACKLIGHT);
	}
        return 0;
}

/* ---------------------------------------------------------
 * lcd_write_string ( fd, str, line )
 * fd:   file handle
 * str:  char string
 * line: what line to print on
 * -------------------------------------------------------- */
int lcd_write_string( int fd, char *str, int line)
{
     int ix, len;
     char lch[]={ 0x80, 0xC0, 0x94, 0xD4 };
     int ich;
     switch (line) {
       case 1:
          lcd_write_char(fd, 0x80, 0);
	  ich=0;
          break;
       case 2:
          lcd_write_char(fd, 0xC0, 0);
	  ich=1;
          break;
       case 3:
          lcd_write_char(fd, 0x94, 0);
	  ich=2;
          break;
       case 4:
          lcd_write_char(fd, 0xD4, 0);
	  ich=3;
          break;
       default:
          lcd_write_char(fd, 0x80, 0);
	  ich=0;
          break;
     }

     len = strlen(str);
     for (ix=0; ix<len; ix++) {
	if ((ix > 0) && ((ix % 20) == 0)) {
              ich++;
	      if (ich > 3) ich=0;
              lcd_write_char(fd, lch[ich], 0);
	}
        lcd_write_char(fd, str[ix], Rs);
     }
     return 0;
}

int lcd_read_byte_data(int fd, char *buf, int cnt)
{
    int len;
    int ix;
    struct timespec _500ms;
    _500ms.tv_sec = 0;
    _500ms.tv_nsec = 2000000L;

    char val=0b00000111;
    write(fd, &val, 1);
    nanosleep(&_500ms, NULL);
    len = read(fd, buf, cnt);
    printf("%d 0x%x %s\n", len, buf[0], buf);

    val=0b00000110;
    write(fd, &val, 1);
    nanosleep(&_500ms, NULL);
    len = read(fd, buf, cnt);
    printf("%d 0x%x\n", len, buf[0]);
}

int lcd_load_custom_chars(int fd, int nchars, char **fontdata) 
{
    int irow, icol;
    char ch;
    lcd_write(fd, LCD_SETCGRAMADDR);
    for (irow=0; irow<nchars; irow++) {
    	for (icol=0; icol<8; icol++) {
		ch = fontdata1[irow][icol];
    		lcd_write_char(fd, ch,  0);
	}
    }
    return 1;

}

int lcd_display_string_pos(int fd, char *str, int line, int pos)
{
   char pos_new;
   int i,len;
   len = strlen(str);
   if (line == 1) pos_new = (char)pos;
   else if (line == 2) pos_new = 0x40 + (char)pos;
   else if (line == 3) pos_new = 0x14 + (char)pos;
   else if (line == 4) pos_new = 0x54 + (char)pos;
   lcd_write_char(fd, 0x80 + pos_new, 0);
   for (i=0; i<len; i++) {
       lcd_write_char(fd, str[i], 1);
   }
   return 1;
}

