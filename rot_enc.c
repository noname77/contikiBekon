#include "contiki.h"
#include "dev/leds.h"
#include "kbi_events.h"
#include "menu.h"

#include <stdio.h>
#include <string.h>

#define FLASH_LED(l) {      \
    leds_on(l);             \
    clock_delay_msec(50);   \
    leds_off(l);            \
    clock_delay_msec(50);}

/*---------------------------------------------------------------------------*/
PROCESS(rotary_enc_process, "Rotary encoder process");
/*---------------------------------------------------------------------------*/

static int rotary_counter;
static char tmp[5];
static char cnt;

void reset_rotary_counter()
{
  rotary_counter = 0;
}

static void T9Callback()
{
  //printf("b");
  //printf("blue");
  leds_on(LEDS_BLUE);
  clock_delay_msec(1);
  leds_off(LEDS_BLUE);

  tmp[cnt++] = 'b';

  if (cnt == 4)
  {
    cnt = 0;
    if (strstr(tmp, "ybyb") != NULL)
    {
      //printf("tmp: %s\n\n", tmp);
      rotary_counter++;
      //printf("counter: %d\n\r", rotary_counter);
      
      memset(tmp, '\0', 5);
      menu_increase();
      print_menu();
    }
  }
}

static void T10Callback()
{
  //printf("y");
  //printf("yellow");
  leds_on(LEDS_YELLOW);
  clock_delay_msec(1);
  leds_off(LEDS_YELLOW);

  tmp[cnt++] = 'y';
  if (cnt == 4)
  {
    cnt = 0;
    if (strstr(tmp, "byby") != NULL)
    {
      //printf("tmp: %s\n\n", tmp);
      rotary_counter--;
      //printf("counter: %d\n\r", rotary_counter);
      
      menu_decrease();
      print_menu();
    }
  }
}

PROCESS_THREAD(rotary_enc_process, ev, data) {
PROCESS_BEGIN();

  leds_off(LEDS_ALL);
  cnt = 0;

  // Set callbacks for events on T9 and T10.
  // The callback can be NULL
  //
  kbi_event_setHandler(T9,  T9Callback);
  kbi_event_setHandler(T10, T10Callback);

/*
  while (1) {
    printf("enc process\n");
    PROCESS_WAIT_EVENT();
    // Loop just in case something other than the alarm
    // we're expecting woke us up
    //
    while (!kbi_event_happened())
        FLASH_LED(LEDS_GREEN);
  }
*/

  PROCESS_END();
}


void rot_enc_init()
{
  process_start(&rotary_enc_process, NULL);
}

void rot_enc_end()
{
  process_exit(&rotary_enc_process);
}
