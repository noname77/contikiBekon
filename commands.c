#include "contiki.h"
#include "mc1322x.h"
#include "shell.h"
#include "commands.h"
#include "uart2_handler.h"
#include "net/rime.h"
//#include "collect_my.h"
//#include "multihop_my.h"
#include "mesh_my.h"
#include "rudolph1_my.h"
#include "dev/serial-line.h"
#include "dev/leds.h"
#include "lcd.h"
#include "ble.h"
#include "pwm.h"
#include "buzzer.h"
#include "clock_my.h"
#include "misc.h"
#include "io-pins.h"
//#include "audio.h"
#include "sys/rtimer.h"

#include <stdio.h>
#include <string.h>



#ifndef WITH_BLE
#define WITH_BLE 0
#endif

#ifndef WITH_LCD
#define WITH_LCD 0
#endif

#ifndef WITH_BUZZ
#define WITH_BUZZ 0
#endif

#ifndef WITH_PROX
#define WITH_PROX 0
#endif

PROCESS(testcmd_process, "Test command");
SHELL_COMMAND(test_command, "test", "test: test own commands", &testcmd_process);
/* --------------------------------- */
PROCESS_THREAD(testcmd_process, ev, data) {
PROCESS_BEGIN();
  //shell_output_str(&test_command, "test: ", "Stuff works!");
  static char* str;
  str = (char*) data;
  char* next;
  
  char* shell_out;

  if ((next = strstr(str, "-s")) != NULL)
  {
    char cmd = 0x0f;
    char addr; 
    for (addr = 0; addr < 120; addr++)
    {
      printf("sending to: 0x%x", addr);
      i2c_transmitinit(addr,1,&cmd);
      while(!i2c_transferred()) /* Wait for transfer */ ;
      clock_delay_msec(10);
    }
  }
  else if ((next = strstr(str, "-i")) != NULL)
  {
    static char result;
    i2c_transmitinit(0x68,1,"\0x75");
    while(!i2c_transferred()) /* Wait for transfer */ ;
    //clock_delay_msec(10);
    i2c_receiveinit(0x68, 1,&result);
    printf("whoami val: %x", result);
  }
  //i2c_transmitinit(acc_addr,1,&cmd);
  


PROCESS_END();
}

/*ifndef WITH_BLE
PROCESS(timer_process, "Rtimer Test command");
SHELL_COMMAND(rtest_command, "rtest", "rtest: test rtimer", &timer_process);
/* --------------------------------- 
PROCESS_THREAD(timer_process, ev, data) {
PROCESS_BEGIN();
  static struct etimer et;

  while(1)
  {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
    printf("now: %d\n\r", RTIMER_NOW());
    //shell_output_str(&test_command, "test: ", "Stuff works!");
  }
PROCESS_END();
}
*/

PROCESS(led_process, "LED process");
SHELL_COMMAND(led_command, "led", "led <color> <on/off>: turn <o/g/b> led on or off", &led_process);
/* --------------------------------- */
PROCESS_THREAD(led_process, ev, data) {
PROCESS_BEGIN();
  char* str = data;
  char l;
  char* next;

  if (*str == 'y') l = LEDS_YELLOW;
  else if (*str == 'g') l = LEDS_GREEN;
  else if (*str == 'b') l = LEDS_BLUE;
  else PROCESS_EXIT();
  
  next = strchr(str, ' ');
  if (next++ == NULL){
    leds_on(l);
    clock_delay_msec(50);
    leds_off(l);
    clock_delay_msec(50);
    PROCESS_EXIT();
  }
     
  if (strcmp(next, "on") == 0) leds_on(l);
  else if (strcmp(next, "off") == 0) leds_off(l);

  shell_output_str(&led_command, "LED OK", NULL);
PROCESS_END();
}

#if WITH_LCD
PROCESS(lcd_process, "LCD command");
SHELL_COMMAND(lcd_command, "lcd", "lcd: print text to lcd", &lcd_process);
/* --------------------------------- */

PROCESS_THREAD(lcd_process, ev, data) {
PROCESS_BEGIN();
  static char* str;
  str = (char*) data;
  char* next;
  
  char* shell_out;

  if ((next = strstr(str, "-C")) != NULL)
  {
    lcd_clear();
    shell_out = "cleared";
  }
  else if ((next = strstr(str, "-i")) != NULL)
  {
    lcd_init();
    shell_out = "initialized";
  }
  else if ((next = strstr(str, "-l")) != NULL)
  {
    next += 3;
    if(*next == '0')
    {
      lcd_backlight(0);
      shell_out = "backlight off";
    }
    else
    {
      lcd_backlight(1);
      shell_out = "backlight on";
    }
  }
  else if ((next = strstr(str, "-h")) != NULL)
  {
    lcd_home();
    shell_out = "cursor homed";
  }
  else if ((next = strstr(str, "-b")) != NULL)
  {
    next += 3;
    if(*next == '0')
    {
      lcd_blink(0);
      shell_out = "cursor blink off";
    }
    else
    {
      lcd_blink(1);
      shell_out = "cursor blink on";
    }
  }
  else if ((next = strstr(str, "-c")) != NULL)
  {
    next += 3;
    if(*next == '0')
    {
      lcd_cursor(0);
      shell_out = "cursor off";
    }
    else
    {
      lcd_cursor(1);
      shell_out = "cursor on";
    }
  }
  else if ((next = strstr(str, "-p1")) != NULL)
  {
    if(strlen(next) > 4)
    {
      next += 4;
      lcd_home();
      lcd_print(next);
      shell_out = "printed";
    }
  }
  else if ((next = strstr(str, "-p2")) != NULL)
  {
    if(strlen(next) > 4)
    {
      next += 4;
      lcd_set_cursor(1,0);
      lcd_print(next);
      shell_out = "printed";
    }
  }

  shell_output_str(&lcd_command, "lcd: ", shell_out);
PROCESS_END();
}
#endif


#if WITH_BLE
//ble control TODO: make it nicer... handle arbitrary many data
PROCESS(ble_cmd_process, "BLE command");
SHELL_COMMAND(ble_command, "ble", "ble: send data to bluetooth", &ble_cmd_process);
// add usage: -s -> serial send, -D1 -> send data1, -d1 -> change data1, -D2 -> send data2, -d2 -> change data2
// now assumes serial send by default
/* --------------------------------- */
#define BUF_LEN 256
PROCESS_THREAD(ble_cmd_process, ev, data) {
PROCESS_BEGIN(); 
  static char buffer[BUF_LEN];
  char i = 0;
  buffer[0] = '\0';

  char* str = (char*) data;
  char* next;
  char len;
  char cmd = 0;

  if(strstr(str, "-r") != NULL)
  {
    ble_reset();
    shell_output_str(&ble_command, "ble: restarted", "");
    PROCESS_EXIT();
  }
  else if (strstr(str, "booted") != NULL)
  {
    shell_output_str(&ble_command, "ble: booted", "");
  }
  else if (strstr(str, "disconnected") != NULL)
  {
    leds_off(LEDS_GREEN);
  }
  else if(strstr(str, "connected") != NULL)
  {
    leds_on(LEDS_GREEN);
  }
  else if (strstr(str, "-dfu") != NULL) //update ble112 firmware from file
  {
    // start the dfu process
    ble_dfu_init();  
    PROCESS_EXIT();
  }
  else if ((next = strstr(str, "-s")) != NULL)
  {
    next += 3;
    len = strlen(next);// < 120 ? strlen(next) : 119;
    cmd = '$';
  }
  //next = strstr(str, "-D1");  // for data1 request
  else if (strstr(str, "-D1") != NULL)
  {
    len = file_read("d1.txt", &buffer[3], BUF_LEN-5, 0);
    cmd = '!';
  }

  if (cmd != 0)
  {
    // prepare buffer
    buffer[i++] = 'S'; // for start
    buffer[i++] = len + 1; // + 1 + ((len - 1) / 63); // length of data with new line terminations (max actual data: 256 - 4 - 3 = 249)
    buffer[i++] = cmd; // for $erial data
    if (cmd == '$') memcpy(&buffer[i], next, len);
    i+=len;
    buffer[i++] = 0x04; // end of transmission
    buffer[i] = '\0';

    static unsigned int buf_len;  //char fucks things up...
    static char buf[64];
    static char* ptr;
    static char cnt;

    cnt = 0;
    buf_len = strlen(buffer);
    ptr = buffer;

    while (buf_len > 0)
    { 
      char size = (cnt == 0) ? 63 : 60;
      if (buf_len > size)
      {
        memcpy(buf, ptr, size);
        buf[size] = '\0';
        buf_len -= size;
        ptr += size;
        cnt++;
      }
      else
      {
        memcpy(buf, ptr, buf_len);
        buf[buf_len] = '\0';
        buf_len = 0;
      } 
      process_post(PROCESS_BROADCAST, UART2_SEND, buf);
      PROCESS_WAIT_EVENT_UNTIL(ev == serial_line2_event_message && strstr(data, "ble x") != NULL);
    }
  }
  //shell_output_str(&ble_command, "ble: OK", "");
PROCESS_END();
}
#endif

#if WITH_BUZZ
PROCESS(buzz_cmd_process, "BUZZ command");
SHELL_COMMAND(buzz_command, "buzz", "buzz <on/off>: turn buzzer on/off", &buzz_cmd_process);
/* --------------------------------- */
PROCESS_THREAD(buzz_cmd_process, ev, data) {
PROCESS_BEGIN(); 

  char* str = (char*) data;
  char* next;
  int freq = 2000;

  if((next = strstr(str, "-f")) != NULL)  // set frequency
  {
    next += 3;
    freq = atoi(next);
  }
  else if((next = strstr(str, "-t"))!= NULL)  // set tone
  {
    next += 3;
    char* last = strchr(next, ' ');
    *last = '\0';
    freq = ttof(next);
    printf("freq: %d\n\r", freq);
    *last = ' ';
    // set tone to play, i.e. c, d, e ...
  }
  else if((next = strstr(str, "-R"))!= NULL)
  {
    next += 3;
    if(*next == '0')
      ringtone_end(); 
    else
      ringtone_init();
  }
  else if((next = strstr(str, "-r"))!= NULL)  // set tone
  {
    next += 3;
    process_post(&ringtone_process, ringtone_play_event, next);
    PROCESS_EXIT();
  }
  
  if(strstr(str, "on") != NULL)
    pwm_init(TMR1,freq,32768);
  else if (strstr(str, "off") != NULL)
    pwm_init_stopped(TMR1,freq,0);

PROCESS_END();
}
#endif

#if WITH_LCD
PROCESS(clock_cmd_process, "Clock command");
SHELL_COMMAND(clock_command, "clock", "clock <on/off>: turn clock display on/off", &clock_cmd_process);
/* --------------------------------- */
PROCESS_THREAD(clock_cmd_process, ev, data) {
PROCESS_BEGIN(); 

  char* str = (char*) data;

  if(strstr(str, "on") != NULL)
    clock_my_init();
  else if (strstr(str, "off") != NULL)
    clock_end();

PROCESS_END();
}
#endif

#if WITH_PROX
PROCESS(prox_cmd_process, "proximity command");
SHELL_COMMAND(prox_command, "prx", "srv <on/off>: nurn proximity sensor process on/off", &prox_cmd_process);
/* --------------------------------- */
PROCESS_THREAD(prox_cmd_process, ev, data) {
PROCESS_BEGIN(); 

  char* str = (char*) data;

  if(strstr(str, "on") != NULL)
    prox_init();
  else if (strstr(str, "off") != NULL)
    prox_end();

PROCESS_END();  
}
#endif

//PROCESS(player_process, "Player command");
//SHELL_COMMAND(player_command, "plyr", "plyr: play PCM files", &player_process);
/* --------------------------------- 
PROCESS_THREAD(player_process, ev, data) {
PROCESS_BEGIN(); 

  char* str = (char*) data;
  char* next;

  if (strstr(str, "-i") != NULL)  // initialize the module
  {
    audio_init();
    shell_output_str(&player_command, "plyr: initialised", "");
  }
  else if((next = strstr(str, "-f")) != NULL)  // choose file to play
  {
    next +=3;
    process_post(&audio_process, player_load_file_event, next);
    shell_output_str(&player_command, "plyr: file loaded", "");
  }

  if(strstr(str, "play") != NULL)
  {
    process_post(&audio_process, player_start_playback_event, NULL);
    shell_output_str(&player_command, "plyr: playing", "");
  }
  else if (strstr(str, "pause") != NULL)
  {
    process_post(&audio_process, player_pause_playback_event, NULL);
    shell_output_str(&player_command, "plyr: paused", "");
  }
  else if (strstr(str, "stop") != NULL)
  {
    audio_stop();
    shell_output_str(&player_command, "plyr: stopped", "");
  }

PROCESS_END();
}
*/

//uart 2 send
// TODO: move to u2 command
PROCESS(uart2_send, "uart2 send command");
SHELL_COMMAND(uart2_send_command, "u2s", "u2s: send string to uart2", &uart2_send);
/* --------------------------------- */
PROCESS_THREAD(uart2_send, ev, data) {
PROCESS_BEGIN();
  shell_output_str(&uart2_send_command, "u2s: sending..", "");
  process_post(PROCESS_BROADCAST, UART2_SEND, data);
PROCESS_END();
}

// change rime address (only works for 2byte long addr atm, 1 character long each (0-9))
// TODO: move to settings cmd
PROCESS(rimeaddr_change_process, "Change rime address proc");
SHELL_COMMAND(rimeaddr_change, "cra", "cra: change rime address", &rimeaddr_change_process);
/* --------------------------------- */
PROCESS_THREAD(rimeaddr_change_process, ev, data) {
PROCESS_BEGIN();
  unsigned char* ptr = data;
  char shell_out[3];  
  //set node address
  rimeaddr_t my_addr;
  my_addr.u8[0] = (*ptr++) - 48;
  my_addr.u8[1] = (*ptr) - 48;
  rimeaddr_set_node_addr(&my_addr);
  sprintf(shell_out, "%d.%d", my_addr.u8[0], my_addr.u8[1]);
  shell_output_str(&rimeaddr_change, "Rime address changed to: ", shell_out);
PROCESS_END();
}

// start collect process
/*
PROCESS(collect_start_process, "Start collect process");
SHELL_COMMAND(collect_start, "cs", "cs: start collect process", &collect_start_process);
 --------------------------------- 
PROCESS_THREAD(collect_start_process, ev, data) {
PROCESS_BEGIN();
  collect_init();
  printf("Collect process started\n\r");    // put that to  collect init
PROCESS_END();
}
*/

// end collect process
/*
PROCESS(collect_end_process, "End collect process");
SHELL_COMMAND(collect_end, "ce", "ce: end collect process", &collect_end_process);
 --------------------------------- 
PROCESS_THREAD(collect_end_process, ev, data) {
PROCESS_BEGIN();
  collect_exit();
  printf("Collect process terminated\n\r");
PROCESS_END();
}
*/

// start multihop process
/*
PROCESS(multihop_start_process, "Start multihop process");
SHELL_COMMAND(multihop_start, "ms", "ms: start multihop process", &multihop_start_process);
 --------------------------------- 
PROCESS_THREAD(multihop_start_process, ev, data) {
PROCESS_BEGIN();
  multihop_init();
  printf("Multihop process started\n\r");
PROCESS_END();
}
*/

// end multihop process
/*
PROCESS(multihop_end_process, "End multihop process");
SHELL_COMMAND(multihop_end, "me", "me: end multihop process", &multihop_end_process);
 --------------------------------- 
PROCESS_THREAD(multihop_end_process, ev, data) {
PROCESS_BEGIN();
  multihop_exit();
  printf("Multihop process terminated\n\r");
PROCESS_END();
}
*/

// multihop send
/*
PROCESS(multihop_send_process, "Multihop send process");
SHELL_COMMAND(multihop_send_cmd, "msnd", "msnd <addr> <data>: send via multihop to [addr]", &multihop_send_process);
 --------------------------------- 
PROCESS_THREAD(multihop_send_process, ev, data) {
PROCESS_BEGIN();
  process_post(&multihop_process, mh_send, data);
  printf("Multihop send requested.\n\r");
PROCESS_END();
}
*/

// mesh control
// TODO: move to mesh command
PROCESS(mesh_cmd_process, "Mesh cmd process");
SHELL_COMMAND(mesh_cmd, "mesh", "mesh <flags> <addr> <data>: control mesh networking", &mesh_cmd_process);
/* --------------------------------- */
PROCESS_THREAD(mesh_cmd_process, ev, data) {
PROCESS_BEGIN();
  static int data_len;
  char* str = data;
  char* next;
  char* shell_out;

  if(str == NULL)
    PROCESS_EXIT();

  if(strstr(str, "-i") != NULL)
  {
    mesh_init();
    shell_out = "started";    
  }
  else if(strstr(str, "-e") != NULL)
  {
    mesh_exit();
    shell_out = "terminated";
  }
  else if((next = strstr(str, "-sp")) != NULL)
  {
    next += 4;

    rimeaddr_t addr;
    addr.u8[0] = *next++ - 48;
    addr.u8[1] = *next - 48;
    mesh_set_dest(addr);

    next += 2;

    if (next != NULL)
    {
      process_post(&mesh_process, mesh_send_event, next);
      PROCESS_WAIT_EVENT_UNTIL(ev == mesh_sent);  // should i wait a little too?
      //delay?
      clock_delay_msec(10);
      process_post(&mesh_process, mesh_send_event, "post");
    }
  }
  else if((next = strstr(str, "-s")) != NULL)
  {
    next += 3;

    rimeaddr_t addr;
    addr.u8[0] = *next++ - 48;
    addr.u8[1] = *next - 48;
    mesh_set_dest(addr);

    next += 2;

    if (next != NULL)
      process_post(&mesh_process, mesh_send_event, next);
    
    shell_out = "sent";
  }
  PROCESS_WAIT_EVENT_UNTIL(ev == mesh_sent);  // should i wait a little too?
  shell_output_str(&mesh_cmd, "mesh: ", shell_out);
PROCESS_END();
}

/*
//rudolph1 command process
PROCESS(rudolph1_cmd_process, "rudolph1 send process");
SHELL_COMMAND(rudolph1_cmd, "r1", "r1 <params>: send <filename> via rudolph1 to <addr>", &rudolph1_cmd_process);
/* --------------------------------- 
PROCESS_THREAD(rudolph1_cmd_process, ev, data) {

PROCESS_BEGIN();

  static struct etimer et;
  char* str = data;
  static char* next;
  char* shell_out1;
  char* shell_out2;

  shell_out1 = "r1: ";

  if(str == NULL)
    PROCESS_EXIT();

  if(strstr(str, "-i") != NULL)
  {
    rudolph1_init();
    shell_out1 = "started";    
  }
  else if(strstr(str, "-e") != NULL)
  {
    rudolph1_exit();
    shell_out1 = "terminated";
  }
  else if((next = strstr(str, "-f")) != NULL)
  {
    next += 3;

    rudolph1_set_filename(next);
    shell_out1 = "r1: save to ";    
    shell_out2 = next;
  }
  // TODO: allow for different filename at destination
  else if((next = strstr(str, "-s")) != NULL)
  {
    next += 3;

    rimeaddr_t addr;
    addr.u8[0] = *next++ - 48;
    addr.u8[1] = *next - 48;
    mesh_set_dest(addr);

    next += 2;

    // activate remote node's rudolph process and set its filename

    static char rbuf[30];
    strncpy(rbuf, "r1 -f ", 6);
    strncpy(&rbuf[6], next, strlen(next));
    rbuf[6+strlen(next)] = '\0';

    printf("rbuf: %s\n\r", rbuf);
    
    //need while so that wait event works (i guess?)
    while(1)
    {
      //shell_input();
      process_post(&mesh_process, mesh_send_event, rbuf);
      PROCESS_WAIT_EVENT_UNTIL(ev == mesh_sent);    // does it work?
      ev = PROCESS_EVENT_NONE;
      process_post(&mesh_process, mesh_send_event, "post");
      PROCESS_WAIT_EVENT_UNTIL(ev == mesh_sent);

      //wait a sec to allow other node to setup
      etimer_set(&et, CLOCK_SECOND/2);
      PROCESS_WAIT_UNTIL(etimer_expired(&et));

      process_post(&rudolph1_process, rudolph1_send_event, next);
      
      printf("rudolph1 send requested.\n\r");
      PROCESS_EXIT();
    }
  }
  shell_output_str(&rudolph1_cmd, shell_out1, shell_out2);
PROCESS_END();
}
*/


// start rudolph1 process
// TODO: move to rudolph1 command
/* 
PROCESS(rudolph1_start_process, "Start rudolph1 process");
SHELL_COMMAND(rudolph1_start, "r1s", "r1s: start rudolph1 process", &rudolph1_start_process);
--------------------------------- 
PROCESS_THREAD(rudolph1_start_process, ev, data) {
PROCESS_BEGIN();
  rudolph1_init();
  printf("rudolph1 process started\n\r");
PROCESS_END();
}

// end rudolph1 process
PROCESS(rudolph1_end_process, "End rudolph1 process");
SHELL_COMMAND(rudolph1_end, "r1e", "r1e: end rudolph1 process", &rudolph1_end_process);
 --------------------------------- 
PROCESS_THREAD(rudolph1_end_process, ev, data) {
PROCESS_BEGIN();
  rudolph1_exit();
  printf("rudolph1 process terminated\n\r");
PROCESS_END();
}

//rudolph1 send process
PROCESS(rudolph1_send_process, "rudolph1 send process");
SHELL_COMMAND(rudolph1_send_cmd, "r1snd", "r1snd <addr> <filename>: send <filename> via rudolph1 to <addr>", &rudolph1_send_process);
/* --------------------------------- 
PROCESS_THREAD(rudolph1_send_process, ev, data) {

PROCESS_BEGIN();

  static struct etimer et;
  char* str = data;
  static char* next;
  if(str == NULL)
    PROCESS_EXIT();

  next = strchr(str, ' ');
  if (next++ == NULL)
    PROCESS_EXIT();

// activate remote node's rudolph process and set its filename

  rimeaddr_t addr;
  addr.u8[0] = *str++ - 48;
  addr.u8[1] = *str - 48;
  mesh_set_dest(addr);

  static char rbuf[30];
  strncpy(rbuf, "r1sf ", 5);
  //char* ptr = rbuf+5;  
  strncpy(&rbuf[5], next, strlen(next));
  rbuf[5+strlen(next)] = '\0';

  printf("rbuf: %s\n\r", rbuf);
  
  //need while so that wait event works (i guess?)
  while(1)
  {
    process_post(&mesh_process, mesh_send_event, rbuf);
    PROCESS_WAIT_EVENT_UNTIL(ev == mesh_sent);    // does it work?
    ev = PROCESS_EVENT_NONE;
    process_post(&mesh_process, mesh_send_event, "post");
    PROCESS_WAIT_EVENT_UNTIL(ev == mesh_sent);

    //wait a sec to allow other node to setup
    etimer_set(&et, CLOCK_SECOND/2);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));

    process_post(&rudolph1_process, rudolph1_send_event, next);

    printf("rudolph1 send requested.\n\r");
    PROCESS_EXIT();
  }
PROCESS_END();
}

//rudolph1 set file
/*
PROCESS(rudolph1_set_file_process, "rudolph1 set file process");
SHELL_COMMAND(rudolph1_set_file, "r1sf", "r1sf: set rudolph1 file", &rudolph1_set_file_process);
 --------------------------------- 
PROCESS_THREAD(rudolph1_set_file_process, ev, data) {
PROCESS_BEGIN();
  rudolph1_set_filename(data);
  printf("rudolph1 will save to: %s\n\r", data);
PROCESS_END();
}
*/


// UART1 control

PROCESS(uart1_cmd_process, "UART1 command process");
SHELL_COMMAND(uart1_cmd, "u1", "u1 <params> <1/0>: configure uart1",&uart1_cmd_process);
/* --------------------------------- */
PROCESS_THREAD(uart1_cmd_process, ev, data) {
PROCESS_BEGIN();
  static char* str;
  str = (char*) data;
  char* next;
  char* shell_out;
  
  if ((next = strstr(str, "-s")) != NULL)
  {
    next += 3;
    if(*next == '0')
    {
      uart1_set_input(NULL);
      shell_out = "shell input disabled";
    }
    else
    {
      // TODO: clear buffers before enabling?
      uart1_set_input(serial_line_input_byte);
      shell_out = "shell input enabled";
    }
  }
  shell_output_str(&uart1_cmd, "uart1: ", shell_out);
PROCESS_END();
}

// UART2 control
PROCESS(uart2_cmd_process, "UART2 command process");
SHELL_COMMAND(uart2_cmd, "u2", "u2 <params> <1/0>: configure uart2",&uart2_cmd_process);
/* --------------------------------- */
PROCESS_THREAD(uart2_cmd_process, ev, data) {
PROCESS_BEGIN();
  static char* str;
  str = (char*) data;
  char* next;
  char* shell_out;  

  if ((next = strstr(str, "-s")) != NULL)
  {
    next += 3;
    if(*next == '0')
    {
      uart2_serial_input_exit();
      shell_out = "shell input disabled";
    }
    else
    {
      // TODO: clear buffers before enabling?
      uart2_serial_input_init();
      shell_out = "shell input enabled";
    }
  }
  if ((next = strstr(str, "-l")) != NULL)
  {
    next += 3;
    if(*next == '0')
    {
      uart2_log_exit();
      shell_out = "log disabled";
    }
    else
    {
      uart2_log_init();
      shell_out = "log enabled";
    }
  }
  shell_output_str(&uart1_cmd, "uart2: ", shell_out);
PROCESS_END();
}

void
commands_init()
{
  // contiki commands
  shell_reboot_init();

  // own commands
  shell_register_command(&test_command);
//shell_register_command(&rtest_command);
  shell_register_command(&led_command);
#if WITH_LCD
  shell_register_command(&lcd_command);
  shell_register_command(&clock_command);
#endif

#if WITH_BLE
  shell_register_command(&ble_command);
#endif

#if WITH_BUZZ
  shell_register_command(&buzz_command);
#endif

#if WITH_PROX
  shell_register_command(&prox_command);
#endif
  //shell_register_command(&player_command);
  shell_register_command(&uart2_send_command);
  shell_register_command(&rimeaddr_change);
  shell_register_command(&mesh_cmd);
  //shell_register_command(&rudolph1_cmd);
  shell_register_command(&uart1_cmd);
  shell_register_command(&uart2_cmd); 
//  shell_register_command(&collect_start);
//  shell_register_command(&collect_end);
//  shell_register_command(&multihop_start);
//  shell_register_command(&multihop_end);
//  shell_register_command(&multihop_send_cmd);

}

