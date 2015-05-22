#include <stdio.h>
#include <string.h>
#include "contiki.h"
#include "mc1322x.h"
#include "contiki-uart.h"
#include "dev/leds.h"
#include "uart2_handler.h"
#include "dev/serial-line.h"
#include "commands.h"
#include "shell.h"

/*---------------------------------------------------------------------------*/
PROCESS(uarts_test, "UART2 TEST");
/*---------------------------------------------------------------------------*/

#define FLASH_LED(l) {leds_on(l); clock_delay_msec(50); leds_off(l); clock_delay_msec(50);}

static char u1tou2;

// tester thread
PROCESS_THREAD(uarts_test, ev, data)
{
  static struct etimer timer;
  static char msg[] = "Data\n";
  char* ptr = msg;
  PROCESS_BEGIN();

  u1tou2 = 0;

  while(1)
  {
    PROCESS_WAIT_EVENT();
    if(ev == serial_line1_event_message)
    {
      FLASH_LED(LEDS_GREEN);
      printf("UART1 received: %s\n\r", data);
      //if(u1tou2)
        //process_start(&uart2_send, data);
    }
    if(ev == serial_line2_event_message)
    {
      printf("UART2 received: %s\n\r", data);
      //for router
      //shell_input(data, strlen(data));
    }
  }

  PROCESS_END();
}

void
uarts_test_init()
{
  process_start(&uarts_test, NULL);
}
