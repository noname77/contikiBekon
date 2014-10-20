// driver for MIDAS MCCOG21605B6W-FPTLWI 16x2 lcd display
// see datasheet here: http://www.farnell.com/datasheets/1663636.pdf
//
// pinout:
// PIN 1 - VOUT         Connect a cap between this and VDD if 3V is used 
// PIN 2 - CAP1N        Conncet a 0.1-4.7 uF cap between this and CAP1P if 3V is used
// PIN 3 - CAP1P
// PIN 4 - VDD          Power supply (3V or 5V) 
// PIN 5 - VSS          GND
// PIN 6 - SDA          i2c data
// PIN 7 - SCL          i2c clock
// PIN 8 - RST          reset

// TODO: be careful with writing commands too fast, it crashes the display for some reason
//       could probably add a delay after each command (like 1ms)

#include "contiki.h"
#include "lcd.h"
//#include <stdio.h>
#include <string.h>

#define LCD_ADDR 0x3e // i2c addr of the display

#define BLINK 0x01
#define CURSOR 0x02
#define DDRAM_SET 0x80
#define LINE2 0x40

static char cursor;
static char line;

void wait_for_transfer()
{
  while(!i2c_transferred()) /* Wait for transfer */ ;
  clock_delay_msec(1);
}

//initialize the display
void lcd_init()
{
  printf("initialize lcd...");

  i2c_enable();
  const char init_commands[] = {0x38, 0x39, 0x14, 0x74, 0x54, 0x6f, 0x0c, 0x01};
  i2c_transmitinit(LCD_ADDR,8,init_commands);
  wait_for_transfer();
  cursor = 0x0f;
  line = 0x00;
  
  lcd_print("hello boss :)");
  lcd_set_cursor(1,0);
  lcd_print("contiki booted!");

  printf("[OK]\n\r");
}

//print string str to display
void lcd_print(char* str)
{
  char len = strlen(str);
  char cmd[len+1];
  cmd[0] = 0x40;
  memcpy(&cmd[1], str, len);
  i2c_transmitinit(LCD_ADDR,len+1,cmd);
  wait_for_transfer();
}

//clear the display
void lcd_clear()
{
  const char cmd[] = {0x80, 0x01};
  i2c_transmitinit(LCD_ADDR,2,cmd);
  wait_for_transfer();
}

//put cursor home (pos 1,1)
void lcd_home()
{
  const char cmd[] = {0x80, 0x02};
  i2c_transmitinit(LCD_ADDR,2,cmd);
  wait_for_transfer();
}

//move cursor to the left (dir = CURSOR_LEFT) or right (dir = CURSOR_RIGHT)
void lcd_move_cursor(char dir)
{
  char cur;
  if (dir == CURSOR_LEFT) cur = 0x10;
  else if (dir == CURSOR_RIGHT) cur = 0x14;

  char cmd[] = {0x80, cur};
  i2c_transmitinit(LCD_ADDR,2,cmd);
  wait_for_transfer();
}

//move cursor to row r(0/1), column c(0-16)
void lcd_set_cursor(int r, int c)
{
  char addr = 0;
  lcd_home();
  if(r != 0) addr |= LINE2;
  if(c<=0x0F) addr += c;
  char cmd[] = {0x80, DDRAM_SET | addr};
  i2c_transmitinit(LCD_ADDR,2,cmd);
  wait_for_transfer();
}

//turn cursor on/off (1/0)
void lcd_cursor(char state)
{
  if (state == 0) cursor &= ~CURSOR;
  else cursor |= CURSOR;
  char cmd[] = {0x80, cursor};
  i2c_transmitinit(LCD_ADDR,2,cmd);
  wait_for_transfer();
}

//turn blink on/off (1/0)
void lcd_blink(char state)
{
  if (state == 0) cursor &= ~BLINK;
  else cursor |= BLINK;
  char cmd[] = {0x80, cursor};
  i2c_transmitinit(LCD_ADDR,2,cmd);
  wait_for_transfer();
}

//delete current character at cursor
void lcd_delete()
{
  const char cmd[] = {0x40, 0x20};
  i2c_transmitinit(LCD_ADDR,2,cmd);
  wait_for_transfer();
}
