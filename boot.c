#include <stdio.h>
#include "contiki.h"
#include "mc1322x.h"
#include "commands.h"
#include "uart2_handler.h"
//#include "uarts_test.h"
#include "collect_my.h"
#include "mesh_my.h"
#include "rudolph1_my.h"
#include "settings_read.h"
#include "ble.h"
#include "lcd.h"
#include "rot_enc.h"
#include "menu.h"
#include "serial-shell.h"
#include "dev/serial-line.h"
#include "io-pins.h"
#include "buzzer.h"

PROCESS(boot_process, "Boot Process");

AUTOSTART_PROCESSES(&boot_process);

PROCESS_THREAD(boot_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Stuff to be done during boot:\n\r");

  // system initialization
  printf("configure io pins...");
  ioPins_configurePin(BLE_RST_PIN, USEGPIO, OUTPUT, NOPULLUP, HYSTERESIS_OFF);
  ioPins_configurePin(LCD_BACKLIGHT_PIN, USEGPIO, OUTPUT, NOPULLUP, HYSTERESIS_OFF);
  ioPins_setValue(BLE_RST_PIN, 0);  // dont boot ble yet
  ioPins_setValue(LCD_BACKLIGHT_PIN, 0);  // lcd backlight off

  printf("[OK]\n\r");
  printf("initialize uart2...");
  uart2_handler_init();
  printf("[OK]\n\r");
  printf("initialize serial line...");
  serial_line_init();
  printf("[OK]\n\r");
  printf("initialize serial shell...");
  set_shell_default_output(UART1);
  serial_shell_init();
  printf("[OK]\n\r");
  printf("initialize file shell...");
  shell_file_init();
  printf("[OK]\n\r");
  printf("register own shell commands...");
  commands_init();
  printf("[OK]\n\r");

/*
  printf("initialize rotary encoder...");
  rot_enc_init();
  printf("[OK]\n\r");
  printf("initialize menu...");
  menu_init();
  printf("[OK]\n\r");
*/

/*
  printf("initialize uSD card...");
	uSDcard_init();
  printf("[OK]\n\r");


  printf("initialize uarts test...");
  uarts_test_init();
  printf("[OK]\n\r");
*/

  //printf("initialize rime collect...");
  //collect_init();
  //printf("[OK]\n\r");

  // initialize network protocols


  printf("initialize rime mesh...");
  mesh_init();
  printf("[OK]\n\r");

/*
  printf("initialize rudolph1...");
  rudolph1_init();
  printf("[OK]\n\r");
*/

/*
  printf("load settings from sd card...\n\r");
  load_settings();
  printf("[OK]\n\r");
*/


  set_shell_default_output(UART1);
  printf("Default shell output is UART1\n\r");

  //uart1_set_input(serial_line_input_byte);
  uart1_set_input(NULL);
  uart2_serial_input_init();
  rimeaddr_t my_addr;
  my_addr.u8[0] = 6;
  my_addr.u8[1] = 6;
  rimeaddr_set_node_addr(&my_addr);
  uart2_log_init();

  ioPins_setValue(8, 1);  // ble reset high
  ble_init();

  ringtone_init();
  static char *boot_song = "2.txt";
  process_post(&ringtone_process, ringtone_play_event, boot_song);
  //lcd_init();
  
  printf("initialize uSD card...");
	uSDcard_init();
  printf("[OK]\n\r");  


  // for amman
  //process_start(&uart1_shell_disable_process, NULL);
  //process_start(&rimeaddr_change_process, "10");
  
  // TODO: register itself in the hub (gateway)

  printf("Boot succesfull.\n\r");  
  PROCESS_END();
}
