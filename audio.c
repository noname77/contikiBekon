// TODO: shell command history (up down arrows)
//       tab - finish command
//       

// audio
// plays 8/16 bit, 8 k samples per second wav (PCM?) files stored on the uSD card
// need to attach a low pass filter and speaker matching inductor on the output pin
// 
// TODO: remember: changed definition of rtimer_t to int (think from short) in rtimer.h

#include <mc1322x.h>
#include "contiki.h"
#include "sys/rtimer.h"
#include "SDcard.h"
#include "pwm.h"


#define FILENAME "holdinon.wav"
#define MAX_TIME
#define CARRIER_FREQ 44000
#define SAMPLING_FREQ 8000
#define SAMPLE_SIZE 8
#define BUF_LEN 100//8000 ram overflow
#define INTERVAL RTIMER_SECOND/SAMPLING_FREQ
//MAX_FILE_LEN MAX_TIME*SAMPLING_FREQ*SAMPLE_SIZE

static char file[BUF_LEN]; //TODO: dynamic memory allocation?
static char* duty;
static struct rtimer my_timer;
static char pause;
static int len;
static int offset;

static void audio_calback(struct rtimer *rt, void* ptr)
{
  printf("test\n\r");
  // modulate the duty cycle depending on the bytes from wav file
  pwm_duty(TMR1, *duty++);
  len++;
  if(len == BUF_LEN)
  {
    // should probably not do it here. need more ram to store the data
    offset += BUF_LEN;
    file_read(FILENAME, file, BUF_LEN, offset);
    duty = file;
  }
  if (!pause)
    rtimer_set(&my_timer, RTIMER_TIME(&my_timer) + INTERVAL, 1, audio_calback, NULL);
}

PROCESS(audio_process, "Audio Process");

process_event_t player_load_file_event;
process_event_t player_start_playback_event;
process_event_t player_pause_playback_event;

PROCESS_THREAD(audio_process, ev, data)
{
PROCESS_BEGIN();

  pause = 0;
  //allocate events
  player_load_file_event = process_alloc_event();
  player_start_playback_event = process_alloc_event();
  player_pause_playback_event = process_alloc_event();

  while(1)
  {
    PROCESS_WAIT_EVENT();
    printf("got an event\n\r");
    // load the song (or part of it) into ram  
    if(ev == player_load_file_event) // TODO: unify event names
    {
      file_read(FILENAME/*(char*) data*/, file, BUF_LEN, 0);
      //printf("loaded\n\r");
      duty = file;
    }
    else if(ev == player_start_playback_event)
    {
      // setup timer (need a callback every every 1/8000 =  1.25 ms for 8000 samples a second
      if(!pause)
      {
        len = 0;
        offset = 0;
        duty = file;
      }
      pause = 0;

      // generate constant 44 kHZ square wave with the duty cycle of PCM file byte value
      pwm_init(TMR1,CARRIER_FREQ,*duty++);
      rtimer_set(&my_timer, RTIMER_NOW() + INTERVAL, 1, audio_calback, NULL);
      //printf("rtimer should be set now: %d\n\r", RTIMER_NOW());
      //printf("interval: %d\n\r", RTIMER_SECOND/SAMPLING_FREQ);
    }
    else if(ev == player_pause_playback_event)
    {
      pause = 1;
      //printf("paused\n\r");
    }
  }
PROCESS_END();
}


void
audio_init()
{
  process_start(&audio_process, NULL);
}

void
audio_stop()
{
  process_exit(&audio_process);
}
