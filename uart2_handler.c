#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "mc1322x.h"
#include "contiki-uart.h"
#include "dev/leds.h"
#include "dev/serial-line.h"
#include "uart2_handler.h"
#include "RTC.h"
#include "cfs/cfs.h"

#define FLASH_LED(l) {leds_on(l); clock_delay_msec(50); leds_off(l); clock_delay_msec(50);}

// define events
process_event_t UART2_SEND;
process_event_t uart2_sent;
/*---------------------------------------------------------------------------*/
PROCESS(uart2_process, "UART2 Process");
/*---------------------------------------------------------------------------*/

// handles uart2 sending and receiving
PROCESS_THREAD(uart2_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  static int i, j;

  leds_off(LEDS_ALL);
  UART2_SEND = process_alloc_event();
  uart2_sent = process_alloc_event();

  while(1)
  {
    PROCESS_WAIT_EVENT();

    //send
    if(ev == UART2_SEND)
    {
      //("SEND\n\r");
      int cnt = 0;
      char* p = data;
      while(*p)
      {
        uart2_putc(*p++);
        cnt++;
      }
      //uart2_putc('\n');
      printf("%d bytes sent to uart2\n\r", cnt);
      process_post(PROCESS_BROADCAST, uart2_sent, NULL);
    }

    //PRINTF("\n\r%d bytes received\n\r", cnt); 
  }

  PROCESS_END();
}


/*---------------------------------------------------------------------------*/
PROCESS(uart2_log_process, "UART2 LOG Process");
PROCESS_THREAD(uart2_log_process, ev, data)
{
  static int fd;
  PROCESS_EXITHANDLER(cfs_close(fd));
  PROCESS_BEGIN();
  
  struct RTC_time t;
  char timestamp[24];

  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(ev == serial_line2_event_message || ev == UART2_SEND);

    // open file to dump uart2 data
    fd = cfs_open("uart2log.txt", CFS_APPEND);
    if(fd < 0) {
      printf("u2le: could not open file for writing");
    }
    else
    {
      //get timestamp
      RTC_getTime(&t);
      sprintf(timestamp, "%02d/%02d/%02d %02d:%02d:%02d.%d%d", t.day, t.month, t.year, t.hours, t.minutes, t.seconds, t.tenths, t.hundredths);

      char direction[] = ",  , data: ";
      if(ev == serial_line2_event_message)
        direction[2] = 'i';
      else
        direction[2] = 'o';

      cfs_write(fd, timestamp, strlen(timestamp));
      cfs_write(fd, direction, strlen(direction));
      cfs_write(fd, data, strlen(data));
      cfs_write(fd, "\n\r", strlen("\n\r"));
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
PROCESS(uart2_serial_input_process, "UART2 Serial Input Process");
/*---------------------------------------------------------------------------*/

// inputs uart2 to the shell
PROCESS_THREAD(uart2_serial_input_process, ev, data)
{
  PROCESS_BEGIN();

  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(ev == serial_line2_event_message);
    shell_input(data, strlen(data));
  }

  PROCESS_END();
}

void
uart2_handler_init()
{
  // Set UART2 to 115200 baud
	//uart2_init(INC, MOD, SAMP);
  //start proces
  process_start(&uart2_process, NULL);
}

void
uart2_log_init()
{
  process_start(&uart2_log_process, NULL);
  printf("uart2 log enabled\n\r");
}

void
uart2_log_exit()
{
  process_exit(&uart2_log_process);
  printf("uart2 log disabled\n\r");
}

void
uart2_serial_input_init()
{
  //uart2_init(63, MOD, SAMP);     //9600
  //uart2_init(NODE_INC, MOD, SAMP); //115200
  // TODO: flush buffers first
  process_start(&uart2_serial_input_process, NULL);
  printf("uart2 shell input enabled\n\r");
}

void
uart2_serial_input_exit()
{
  process_exit(&uart2_serial_input_process);
  printf("uart2 shell input disabled\n\r");
}

