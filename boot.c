#include <stdio.h>
#include "contiki.h"
#include "mc1322x.h"
#include "commands.h"
#include "uart2_handler.h"
#include "uarts_test.h"
#include "collect_my.h"
#include "mesh_my.h"
#include "rudolph1_my.h"
#include "settings_read.h"
#include "lcd.h"

PROCESS(boot_process, "Boot Process");

AUTOSTART_PROCESSES(&boot_process);

PROCESS_THREAD(boot_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Stuff to be done during boot:\n\r");

  // system initialization
  printf("initialize uart2...");
  uart2_handler_init();
  printf("[OK]\n\r");
  printf("initialize serial line...");
  serial_line_init();
  printf("[OK]\n\r");
  printf("initialize serial shell...");
  serial_shell_init();
  printf("[OK]\n\r");
  printf("initialize file shell...");
  shell_file_init();
  printf("[OK]\n\r");
  printf("register own shell commands...");
  commands_init();
  printf("[OK]\n\r");

/*
  printf("initialize uSD card...");
	uSDcard_init();
  printf("[OK]\n\r");
*/

  printf("initialize uarts test...");
  uarts_test_init();
  printf("[OK]\n\r");

  //printf("initialize rime collect...");
  //collect_init();
  //printf("[OK]\n\r");

  // initialize network protocols


  printf("initialize rime mesh...");
  mesh_init();
  printf("[OK]\n\r");

  printf("initialize rudolph1...");
  rudolph1_init();
  printf("[OK]\n\r");
  
  printf("load settings from sd card...");
  load_settings();
  printf("[OK]\n\r");

  // for amman
  //process_start(&uart1_shell_disable_process, NULL);
  //process_start(&rimeaddr_change_process, "10");
  
  // register itself in the hub

  printf("Boot succesfull.\n\r");  
  PROCESS_END();
}
