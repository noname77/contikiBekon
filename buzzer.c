#include "contiki.h"
#include <mc1322x.h>
#include <string.h>
#include "buzzer.h"
#include "io-pins.h"
#include "pwm.h"

// from http://www.phy.mtu.edu/~suits/notefreqs.html
#define C5	523
#define Db5	554
#define D5	587
#define Eb5	622
#define E5	659
#define F5	698
#define Gb5	740
#define G5	784
#define Ab5	831
#define A5	880
#define Bb5	932
#define B5	988
#define C6	1047
#define Db6	1109
#define D6	1175
#define Eb6	1245
#define E6	1319
#define F6	1397
#define Gb6	1480
#define G6	1568
#define Ab6	1661
#define A6	1760
#define Bb6	1865
#define B6	1976

static char* tones[] = {"C5", "Db5", "D5", "Eb5", "E5", "F5", "Gb5", "G5", "Ab5", "A5", "Bb5", "B5", "C6", "Db6", "D6", "Eb6", "E6", "F6", "Gb6", "G6", "Ab6", "A6", "Bb6", "B6"};
static int freqs[] = {C5, Db5, D5, Eb5, E5, F5, Gb5, G5, Ab5, A5, Bb5, B5, C6, Db6, D6, Eb6, E6, F6, Gb6, G6, Ab6, A6, Bb6, B6};

int ttof(char* t)
{
  char i;
  for(i=0; i<24; i++)
    if(strstr(t, (char*)tones[i]) != NULL)
      return freqs[i];
}

struct rtttl_header {
  char* name;  //TODO: how to print till delimiter?
  unsigned char d;
  unsigned char o;
  unsigned char b;
};

process_event_t ringtone_play_event;

// addopted from http://retired.beyondlogic.org/pic/ringtones.htm
// some other info: http://www.electronicsforu.com/electronicsforu/circuitarchives/view_article.asp?sno=461
PROCESS(ringtone_process, "Ringtone Process");

#define BUFF_LEN 256  // maximum RTTTL filesize

PROCESS_THREAD(ringtone_process, ev, data)
{
  PROCESS_BEGIN();
  printf("BUZZ.\n\r");

  ringtone_play_event = process_alloc_event();

  static struct etimer et;
  static unsigned char buff[BUFF_LEN]; // TODO: dynamic allocation?
  char* filename;
  static struct rtttl_header hdr;
  static char* ptr;
  static char ringtone_finished;
  static unsigned int time_interval;
  // configure gpio
  ioPins_configurePin(BUZZ_PIN, USEGPIO, OUTPUT, NOPULLUP, HYSTERESIS_OFF);
  
  while(1)
  {
    //printf("w, e: %d\n\r", ev);
    PROCESS_WAIT_EVENT_UNTIL(ev == ringtone_play_event || ev == PROCESS_EVENT_TIMER);
    if(ev == ringtone_play_event) 
    {
      ringtone_finished = 0;
      filename = (char*) data;
      shell_output_str(NULL, "loading file: ", filename);
      // load ringtone to memory
      file_read(filename, buff, BUFF_LEN, 0);

      // process info -> read name, 

      char* p;
      p = strchr(buff, ':');
      
      // if lcd exists / is initialised TODO: how to check it, and include appriopriate code only then
      // lcd print name
      *p = '\0';
      shell_output_str(NULL, "name:", buff);
      lcd_clear();
      lcd_print("Name:");
      lcd_set_cursor(1,0);
      lcd_print(buff);
      *p = ':';

      // TODO: whats the point of using ifs?
      if(strstr(p, "d=") != NULL)
      {
        p += 3;
        hdr.d = (unsigned char) atoi(p);
        p = strchr(p, ',');
      }
      if(strstr(p, "o=") != NULL)
      {
        p += 3;
        hdr.o = (unsigned char) atoi(p);
        p = strchr(p, ',');
      }
      if(strstr(p, "b=") != NULL)
      {
        p += 3;
        hdr.b = (unsigned char) atoi(p);
        time_interval = CLOCK_SECOND*60/hdr.b;
        p = strchr(p, ':');
      }
      printf("d=%u, o=%u, b=%u\n\r", hdr.d, hdr.o, hdr.b);
      printf("time_interval: %u\n\r", time_interval);
      ptr = ++p;
      etimer_set(&et, CLOCK_SECOND/100);
    }
    else if(ev == PROCESS_EVENT_TIMER) // timer expired
    {
      unsigned int freq;
      unsigned char duration;
      unsigned char octave;

      if(ringtone_finished)
      {
        pwm_init_stopped(TMR1,freq,0);
        continue;
      }
      // process duration
      if (*ptr == '3' && *(ptr+1) == '2')
      {
        duration = 32;
        ptr += 2;
      }
      else if (*ptr == '1' && *(ptr+1) == '6')
      {
        duration = 16;
        ptr += 2;
      }
      else if (*ptr == '8')
      {
        duration = 8;
        ptr++;
      }
      else if (*ptr == '4')
      {
        duration = 4;
        ptr++;
      }
      else if (*ptr == '2')
      {
        duration = 2;
        ptr++;
      }
      else if (*ptr == '1')
      {
        duration = 1;
        ptr++;
      }
      else duration = hdr.d;      

      if (*(ptr+1) == '#')
      {
      /* Process Sharps */
        if(*ptr == 'a') freq = Bb6;
        else if(*ptr == 'c') freq = Db6;
        else if(*ptr == 'd') freq = Eb6;
        else if(*ptr == 'f') freq = Gb6;
        else if(*ptr == 'g') freq = Ab6;
        ptr += 2;
      }
      else 
      {
        if (*ptr == 'a') freq = A6;
        else if (*ptr == 'b') freq = B6;
        else if (*ptr == 'c') freq = C6;
        else if (*ptr == 'd') freq = D6;
        else if (*ptr == 'e') freq = E6;
        else if (*ptr == 'f') freq = F6;
        else if (*ptr == 'g') freq = G6;
        else if (*ptr == 'p') freq = 0;
        
        ptr++;
      }

      if (*ptr == '.')
      {
        /* Duration 1.5x */
        duration = duration + 128;
        ptr++;
      }

      char i;
      for(i = 4; i<8; i++)
      {
        if(*ptr == 48+i)
        {
          octave = i;
          ptr++;
          break;
        }
      }
      if(i == 8) octave = hdr.o;

      if (*ptr == '.')
      {
        /* Duration 1.5x */
        duration = duration + 128;
        ptr++;
      }
      
      // process octaves
      char tmp = 6 - octave;      
      freq = tmp >= 0 ? freq >> tmp : freq << -tmp;
      //printf("freq: %u\n\r", freq);
      // set pwm
      pwm_init(TMR1,freq,32768);

      if (*ptr == ',') // there will be next note
        ptr++;
      else
        ringtone_finished = 1;

      unsigned int period = time_interval/(duration & 0x7F);
      if (duration & 0x80) period += period >> 1;
      // TODO: somethings wrong with the playback speed...
      //printf("period: %u\n\r", period);
      etimer_set(&et, period);
    }
  }
  PROCESS_END();
}

void ringtone_init()
{
  process_start(&ringtone_process, NULL);
  //pwm_init(TMR1,2000,32768);
}

void ringtone_end()
{
  process_exit(&ringtone_process);
  //pwm_init_stopped(TMR1,2000,32768);
}
