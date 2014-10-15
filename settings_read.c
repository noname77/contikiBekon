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

PROCESS(settings_load_process, "Load settings process");

#define FILENAME "settings.txt"
#define SET_RIME "RIMEADDR"
#define SET_COLLECT "COLLECT"
#define SET_MULTIHOP "MULTIHOP"
#define UART1_TO_UART2 "U1_TO_U2"
#define UART1_SHELL_INPUT "UART1_SHELL_INPUT"
#define UART2_LOG "UART2_LOG"

PROCESS_THREAD(settings_load_process, ev, data)
{

PROCESS_BEGIN()
  // Initialise the micro SD card.
  // We must have this statement before using the card
  //
	uSDcard_init();
  printf("Initialised uSD card\n");

  // Obtain a file descriptor for the file, capable of
  // handling both reads and writes.
  //
  int fd = cfs_open(FILENAME, CFS_READ);
  if (fd < 0) {
    printf("Failed to open %s\n", FILENAME);
    PROCESS_EXIT();
  }
  printf("Opened file %s\n", FILENAME);

  // Read the message back
  //
  int file_len = cfs_seek(fd, 0, CFS_SEEK_END);
  file_len = file_len<64?file_len:64;
  cfs_seek(fd, 0, CFS_SEEK_SET);

  char settings[64];
  int r = cfs_read(fd, settings, file_len);

  if (r < file_len) {
    printf("Failed to read %d bytes from %s, got %d bytes\n",
             file_len, FILENAME, r);
      cfs_close(fd);
      return 0;
  }
	settings[r] = '\0';

 	printf("Read settings: %s", settings);

  // Close the file and release resources associated with fd
  //
  cfs_close(fd);

	// Now power down the micro SD card
  //
	uSDcard_power_down();
 	printf("uSD card powered down\n");
  
 //TODO: change settings only if present in the file, otherwise load default

// does it get passed without static?
  static char *ptr;
  ptr = strstr(settings, SET_RIME);
  ptr += strlen(SET_RIME) + 1;

  process_start(&rimeaddr_change_process, ptr);

// autostart collect
/*
  ptr = strstr(settings, SET_COLLECT);
  ptr += strlen(SET_COLLECT) + 1;  
  if(*ptr == '1')
    process_start(&collect_start_process, ptr);
// autostart multihop
  ptr = strstr(settings, SET_MULTIHOP);
  ptr += strlen(SET_MULTIHOP) + 1;  
  if(*ptr == '1')
    process_start(&multihop_start_process, ptr);
*/

//redirect uart1 output traffic to uart2
  ptr = strstr(settings, UART1_TO_UART2);
  ptr += strlen(UART1_TO_UART2) + 1;  
  if(*ptr == '1')
    u1tou2 = 1;

// enable UART1 shell input
  ptr = strstr(settings, UART1_SHELL_INPUT);
  ptr += strlen(UART1_SHELL_INPUT) + 1;  
  if(*ptr == '1')
    process_start(&uart1_shell_enable_process, NULL);
  else
    process_start(&uart1_shell_disable_process, NULL);

// enable UART2 logging
  ptr = strstr(settings, UART2_LOG);
  ptr += strlen(UART2_LOG) + 1;  
  if(*ptr == '1')
    process_start(&uart2_log_enable_process, NULL);
  else
    process_start(&uart2_log_disable_process, NULL);


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
