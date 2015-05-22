#include "contiki.h"
#include <mc1322x.h>
#include "RTC.h"

// print date and time on lcd
PROCESS(clock_process, "Clock Process");
PROCESS_THREAD(clock_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer et;
  struct RTC_time t;
  char date[17];
  char time[17];

  while(1)
  {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));

    // read RTC time into t structure
    RTC_getTime(&t);
    // prepare strings to be printed
    sprintf(date, "    %02d/%02d/%02d    ", t.day, t.month, t.year);
    sprintf(time, "    %02d:%02d:%02d    ", t.hours, t.minutes, t.seconds);

    // print date and time to lcd
    lcd_home();
    lcd_print(date);
    lcd_set_cursor(1,0);
    lcd_print(time);
  }

  PROCESS_END();
}

void clock_my_init()
{
  process_start(&clock_process, NULL);
}

void clock_end()
{
  process_exit(&clock_process);
}
