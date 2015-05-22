/**
 * \file
 *         Load settings from sd card / default.
 * \author
 *         Wiktor Grajkowski <wiktor.grajkowski@gmail.com>
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "ff.h"
#include "commands.h"
#include "uarts_test.h"
#include "lcd.h"
#include "ble.h"
#include "SDcard.h"
#include "serial-shell.h"
#include "dev/serial-line.h"
#include "io-pins.h"

#define FILENAME "settings.txt"
#define MAX_SETTINGS_LEN 128

#define SET_RIME "RIMEADDR"
#define SET_COLLECT "COLLECT"
#define SET_MULTIHOP "MULTIHOP"
#define UART1_TO_UART2 "U1_TO_U2"
#define UART1_SHELL_INPUT "UART1_SHELL_INPUT"
#define UART2_SHELL_INPUT "UART2_SHELL_INPUT"
#define UART2_LOG "UART2_LOG"
#define SETTING_WITH_LCD "WITH_LCD"
#define IS_GATEWAY "IS_GATEWAY"
#define SETTING_WITH_BLE "WITH_BLE"

PROCESS(settings_load_process, "Load settings process");
PROCESS_THREAD(settings_load_process, ev, data)
{

PROCESS_BEGIN()
  // Initialise the micro SD card.
  // We must have this statement before using the card
	uSDcard_init();
  //printf("Initialised uSD card\n");
  char settings[MAX_SETTINGS_LEN];

  file_read(FILENAME, settings, MAX_SETTINGS_LEN, 0);
  
 	printf("Read settings.txt:\n\r%s\n\r", settings);
	// power down the micro SD card
	//uSDcard_power_down();
 	//printf("uSD card powered down\n\r");

// does it get passed without static?
  static char *ptr;

  ptr = strstr(settings, IS_GATEWAY);
  if (ptr != NULL)
  {
    ptr += strlen(IS_GATEWAY) + 1;
    if(*ptr == '1')
    {
      set_shell_default_output(UART2);
      printf("Default shell output is UART2.\n\r");
    }
    else
    {
      set_shell_default_output(UART1);
      printf("Default shell output is UART1\n\r");
    }
  }

  // set rime addr
  ptr = strstr(settings, SET_RIME);
  if (ptr != NULL)
  {
    ptr += strlen(SET_RIME) + 1;
    process_start(&rimeaddr_change_process, ptr);
  }

/*
  // autostart collect
  ptr = strstr(settings, SET_COLLECT);
  if (ptr != NULL)
  {
    ptr += strlen(SET_COLLECT) + 1;
    if(*ptr == '1')
      process_start(&collect_start_process, ptr);
  }
  // autostart multihop
  ptr = strstr(settings, SET_MULTIHOP);
  if (ptr != NULL)
  {
    ptr += strlen(SET_MULTIHOP) + 1;
    if(*ptr == '1')
      process_start(&multihop_start_process, ptr);
  }
*/
  // enable UART1 shell input
  ptr = strstr(settings, UART1_SHELL_INPUT);
  if (ptr != NULL)
  {
    ptr += strlen(UART1_SHELL_INPUT) + 1;  
    if(*ptr == '1')
      uart1_set_input(serial_line_input_byte);
      //process_start(&uart1_shell_enable_process, NULL);
    else
      uart1_set_input(NULL);
      //rocess_start(&uart1_shell_disable_process, NULL);
  }
  // enable UART2 shell input
  ptr = strstr(settings, UART2_SHELL_INPUT);
  if (ptr != NULL)
  {
    ptr += strlen(UART2_SHELL_INPUT) + 1;  
    if(*ptr == '1')
      uart2_serial_input_init();
      //process_start(&uart2_shell_enable_process, NULL);
    else
      uart2_serial_input_exit();
      //process_start(&uart2_shell_disable_process, NULL);
  }
  // enable UART2 logging
  ptr = strstr(settings, UART2_LOG);
  if (ptr != NULL)
  {
    ptr += strlen(UART2_LOG) + 1;  
    if(*ptr == '1')
      shell_input("u2 -l 1", 7);
    else
      shell_input("u2 -l 0", 7);
  }
  // power up ble
  ptr = strstr(settings, SETTING_WITH_BLE);
  if (ptr != NULL) 
  {
    ptr += strlen(SETTING_WITH_BLE) + 1;
    if(*ptr == '1')
    {
      ioPins_setValue(8, 1);  // ble reset high
      ble_init();
    }
  }
  // initialize lcd

  ptr = strstr(settings, SETTING_WITH_LCD);
  if (ptr != NULL) 
  {
    ptr += strlen(SETTING_WITH_LCD) + 1;
    if(*ptr == '1')
      lcd_init();
  }

//redirect uart1 output traffic to uart2
/*
  ptr = strstr(settings, UART1_TO_UART2);
  ptr += strlen(UART1_TO_UART2) + 1;  
  if(*ptr == '1')
    u1tou2 = 1;
*/



PROCESS_END();
}

//settings read process
  //initialize sd card
  // open sd card and read settings.txt file
    // find RIMEADDR and set it node addr
    // close file
  // otherwise load default
  // sleep sd card

// start settings read process

void
load_settings()
{
  process_start(&settings_load_process, NULL);
}
