#include "contiki.h"
#include <mc1322x.h>
#include "mesh_my.h"

PROCESS(prox_process, "proximity sensor process");
/* --------------------------------- */
PROCESS_THREAD(prox_process, ev, data) {
PROCESS_BEGIN();

  char* str = (char*) data;
  static struct etimer et;
  static char state;
  state = 0;
  int val = 0;
  static int cntr1;
  static int cntr2;
  cntr1 = 0;
  cntr2 = 0;
#include "mesh_my.h"
  while (1)
  {
    etimer_set(&et, CLOCK_SECOND/10);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    // Read ADC0
    val = ioPins_getValue(15);

    if (val > 2200 && state == 0)
    {
      cntr1++;
      if (cntr1 > 12)
      {
        state = 2;
        // do something here
        shell_input("led b on", 8);
        shell_input("mesh -sp 66 lcd -C", 18);
        PROCESS_WAIT_EVENT_UNTIL(ev == mesh_sent);
        PROCESS_WAIT_EVENT_UNTIL(ev == mesh_sent);
        shell_input("mesh -sp 66 lcd -p1 magic happens", 33);
      }
    }
    else if (val > 1300 && state == 0)
    {
      cntr2++;
      if (cntr2 > 12)
      {
        state = 1;
        // do something here
        shell_input("led y on", 8);
      }
    }
    else if (val < 1300 && state != 0)
    {
      state = 0;
      cntr1 = 0;
      cntr2 = 0;
      shell_input("led y off", 5);
      shell_input("led b off", 5);
    }

    //printf("ADC0: %04u\r\n", val);
  }
PROCESS_END();  
}

void prox_init()
{
  process_start(&prox_process, NULL);
}

void prox_end()
{
  process_exit(&prox_process);
}
