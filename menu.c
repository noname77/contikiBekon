#include "contiki.h"
#include "mc1322x.h"
#include "button-sensors.h"
#include "rot_enc.h"
#include "pwm.h"
#include "random.h"

#include <string.h>

#define LEVEL_ROOT 0
#define LEVEL_SUBMENU 1
#define LEVEL_ACTION 2

#define MENU_SETTINGS 0
#define MENU_LCD 1
#define MENU_BLE 2
#define MENU_BUZZER 3
#define MENU_CLOCK 4

#define SETTINGS_WRITE          1
#define SETTINGS_SET_RIMEADDR   2
#define SETTINGS_UART1_SHELL    3
#define SETTINGS_UART2_SHELL    4
#define SETTINGS_UART2_LOG      5

#define LCD_BACKLIGHT     1
#define LCD_CLEAR         2
#define LCD_CURSOR        3
#define LCD_CURSOR_BLINK  4

#define BLE_RESET 1

#define BUZZER_PLAY_TONE      1
#define BUZZER_PLAY_FREQ      2
#define BUZZER_RINGTONE_INIT  3
#define BUZZER_RINGTONE_PLAY  4
#define BUZZER_OFF            5

#define CLOCK_SHOW  1
#define CLOCK_SET   2



static char* settings_menu[] = {"settings", "write to SD", "set rimeaddr", "uart1 shell", "uart2 shell", "uart2 log"};
static char* lcd_menu[] = {"lcd", "backlight", "clear", "cursor", "cursor blink"};
static char* ble_menu[] = {"ble", "reset"};
static char* buzzer_menu[] = {"buzzer", "play tone", "play freq","player init", "play ringtone", "off"};
static char* clock_menu[] = {"clock", "show", "set"};

static char** menu[] = {settings_menu, lcd_menu, ble_menu, buzzer_menu, clock_menu};

// TODO: change to defines
static char menu_count = 5;
static char submenu_count[] = {6, 5, 2, 6, 3};

static char menu_level;
static char menu_id;
static char submenu_id;
static char on_off;
static char menu_action;

void print_menu();

void menu_action_on_off(char* name, char* shell_on, char* shell_off)
{
  lcd_print(name);
  lcd_set_cursor(1,0);
  if(on_off == 1)
    lcd_print("   ~on   off    ");
  else
    lcd_print("    on  ~off    ");

  if(menu_action == 1)
  {
    menu_action = 0;
    menu_level--;
    if(on_off == 1)
      shell_input(shell_on, strlen(shell_on));
    else
      shell_input(shell_off, strlen(shell_off));
    print_menu();
  }
}

// TODO: change name to something more adequate, cleanup, move common lines to functions...
void print_menu()
{
  lcd_clear();

  if (menu_level == LEVEL_ROOT)
  {
    submenu_id = 0;
    if(menu_id+1 == menu_count)
    {
      lcd_set_cursor(0,1);
      lcd_print(menu[menu_id-1][submenu_id]);
      lcd_set_cursor(1,0);
      lcd_print("~");
      lcd_print(menu[menu_id][submenu_id]);
    }
    else
    {
      lcd_print("~");
      lcd_print(menu[menu_id][submenu_id]);
      lcd_set_cursor(1,1);
      lcd_print(menu[menu_id+1][submenu_id]);
    }
  }    
  else if (menu_level == LEVEL_SUBMENU)
  {
    if(submenu_id+1 == submenu_count[menu_id])
    {
      lcd_set_cursor(0,1);
      lcd_print(menu[menu_id][submenu_id-1]);
      lcd_set_cursor(1,0);
      lcd_print("~");
      lcd_print(menu[menu_id][submenu_id]);
    }
    else
    {
      submenu_id == 0 ? lcd_print("^") : lcd_print("~");
      lcd_print(menu[menu_id][submenu_id]);
      lcd_set_cursor(1,1);
      lcd_print(menu[menu_id][submenu_id+1]);
    }
  }
  else
  {
    if(menu_id == MENU_SETTINGS)
    {
      if (submenu_id == SETTINGS_WRITE)
      {

      }
      else if (submenu_id == SETTINGS_SET_RIMEADDR)
      {

      }
      else if (submenu_id == SETTINGS_UART1_SHELL)
      {
        menu_action_on_off("uart1 shell:","u1 -s 1","u1 -s 0");
      }
      else if (submenu_id == SETTINGS_UART2_SHELL)
      {
        menu_action_on_off("uart2 shell:","u2 -s 1","u2 -s 0");
      }
      else if (submenu_id == SETTINGS_UART2_LOG)
      {
        menu_action_on_off("uart1 shell:","u2 -l 1","u2 -l 0");
      }
    }
    else if (menu_id == MENU_LCD)
    {
      if (submenu_id == LCD_BACKLIGHT)
      {
        menu_action_on_off("backlight:","lcd -l 1","lcd -l 0");
      }
      else if (submenu_id == LCD_CLEAR)
      {
        lcd_print("press again to ");
        lcd_set_cursor(1,0);
        lcd_print("clear");

        if(menu_action == 1)
        {
          menu_action = 0;
          menu_level--;
          shell_input("lcd -C", 6);
        }
      }
      else if (submenu_id == LCD_CURSOR)
      {
        menu_action_on_off("cursor:","lcd -c 1","lcd -c 0");
      }
      else if (submenu_id == LCD_CURSOR_BLINK)
      {
        menu_action_on_off("cursor blink:","lcd -b 1","lcd -b 0");
      }
    }
    else if (menu_id == MENU_BLE)
    {
      if (submenu_id == BLE_RESET)
      {

      }
    }
    else if (menu_id == MENU_BUZZER)
    {
      if (submenu_id == BUZZER_PLAY_TONE)
      {
 
      }
      else if (submenu_id == BUZZER_PLAY_FREQ)
      {
        printf("test\n\r");
        char buf[6];
        lcd_print("freq:");
        lcd_set_cursor(1,0);
        sprintf(buf,"%d",rotary_counter);
        printf("counter: %d, string: %s\n\r",rotary_counter, buf);
        lcd_print(buf);
        pwm_init(TMR1,rotary_counter,32768);
      }

      else if (submenu_id == BUZZER_RINGTONE_INIT)
      {
        lcd_print("press again to ");
        lcd_set_cursor(1,0);
        lcd_print("initialize.");

        if(menu_action == 1)
        {
          menu_action = 0;
          menu_level--;
          shell_input("buzz -R", 7);
        }
      }
      else if (submenu_id == BUZZER_RINGTONE_PLAY)
      {
        lcd_print("random ringtone:");
        lcd_set_cursor(1,0);
        if(on_off == 1)
          lcd_print(" ~play   cancel ");
        else
          lcd_print("  play  ~cancel ");

        if(menu_action == 1)
        {
          menu_action = 0;
          menu_level--;
          if(on_off == 1)
          {
            #define RINGTONES_COUNT 10403
            static char cmd[18];
            int random = (random_rand() % RINGTONES_COUNT) + 1;
            //choose random one
            sprintf(cmd, "buzz -r %d.txt", random);
            //printf(cmd);
            shell_input(cmd, strlen(cmd));
          }
          print_menu();
        }
      }
      else if (submenu_id == BUZZER_OFF)
      {

      }
    }
    else if (menu_id == MENU_CLOCK)
    {      
      if (submenu_id == CLOCK_SHOW)
      {
        if(menu_action == 1)
        {
          menu_action = 0;
          if(on_off == 0)
          {
            shell_input("clock on", 8);
            on_off++;
          }
          else
          {
            menu_level--;
            shell_input("clock off", 9);
          }
        }
      }
      else if (submenu_id == CLOCK_SET)
      {

      }
    }
  }
}

void menu_select()
{
  if(menu_level != LEVEL_ROOT && submenu_id == 0)
  {
    if (menu_level > 0)
      menu_level--;
  }
  else
  {
    if(menu_level < LEVEL_ACTION)
    {
      menu_level++;
      on_off = 0;
    }
    else
      menu_action = 1;
  }
//  if (menu_level == LEVEL_ACTION)
//    reset_rotary_counter();
}

void menu_increase()
{
  if (menu_level == LEVEL_ROOT)
  {
    if(menu_id < menu_count-1)
      menu_id++;
  }
  else if (menu_level == LEVEL_SUBMENU)
  {
    //printf("submenu_count: %d\n\r", submenu_count[menu_id]);
    if(submenu_id < submenu_count[menu_id]-1)
      submenu_id++;
  }
  else if(menu_level == LEVEL_ACTION)
  {
    on_off = 0;
  }
  //printf("menu_level: %d\n\rmenu_id: %d\n\rsubmenu_id: %d\n\r", menu_level, menu_id, submenu_id);
}

void menu_decrease()
{
  if (menu_level == LEVEL_ROOT)
  {
    if (menu_id > 0)
      menu_id--;
  }
  else if (menu_level == LEVEL_SUBMENU)
  {
    if (submenu_id > 0)
      submenu_id--;
  }
  else if(menu_level == LEVEL_ACTION)
  {
    on_off = 1;
  }
  //printf("menu_id: %d\n\rsubmenu_id: %d\n\r", menu_id, submenu_id);
}


PROCESS(menu_process, "Menu Process");
PROCESS_THREAD(menu_process, ev, data)
{
PROCESS_BEGIN();

  menu_level = LEVEL_ROOT;
  menu_id = 0;
  submenu_id = 0;
  on_off = 1;
  menu_action = 0;

  SENSORS_ACTIVATE(button_sensor);

  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
    //printf("button pressed\n\r");
    menu_select();
    print_menu();
  }
PROCESS_END();
}

void menu_init()
{
  process_start(&menu_process, NULL);
}
