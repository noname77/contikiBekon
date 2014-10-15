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


#include <stdio.h>
#include <string.h>

PROCESS(testcmd_process, "Test command");
SHELL_COMMAND(test_command, "test", "test: test own commands", &testcmd_process);
/* --------------------------------- */
PROCESS_THREAD(testcmd_process, ev, data) {
PROCESS_BEGIN();
  printf("Stuff works!\n\r");
PROCESS_END();
}

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

  if (!strcmp(next, "on")) leds_on(l);
  else if (!strcmp(next, "off")) leds_off(l);
PROCESS_END();
}

PROCESS(lcd_process, "LCD command");
SHELL_COMMAND(lcd_command, "lcd", "lcd: print text to lcd", &lcd_process);
/* --------------------------------- */
PROCESS_THREAD(lcd_process, ev, data) {
PROCESS_BEGIN();
  static struct etimer et;
  static char* str;
  const num = 13;
  static char buf[num+2];  
  static char i = 0;
  static char len;
  str = data;  
  while(1)
  {
    len = strlen(str);
    if (i+num<len)
    {
      memcpy(buf, &str[i], num);
      buf[num]='\0';
      //printf("i: %d num: %d len: %d buffer: %s\n\r",i,num,len,buf);
    }
    else
    {
      memcpy(buf, &str[i], len-i);
      buf[len-i]='\r';
      buf[len-i+1]='\0';
      //printf("buffer-CR: %s\n\r",buf);
    }
    process_post(&uart2_process, UART2_SEND, buf);
    //process_post(PROCESS_BROADCAST, UART2_SEND, buf);
    //PROCESS_YIELD();

    i+=num;
    if(i>len)  {i = 0; PROCESS_EXIT();}
    PROCESS_WAIT_EVENT_UNTIL(ev == uart2_sent);
    etimer_set(&et, CLOCK_SECOND/5);
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
  }
PROCESS_END();
}

//ble control
PROCESS(ble_process, "BLE command");
SHELL_COMMAND(ble_command, "ble", "ble: send data to bluetooth", &ble_process);
// add usage: -s -> serial send, -d1 -> change data1, -d2 -> change data2
// now assumes serial send by default
/* --------------------------------- */
#define BUF_LEN 256
PROCESS_THREAD(ble_process, ev, data) {
PROCESS_BEGIN(); 
  static char buffer[BUF_LEN];
  char i = 0;
  buffer[i++] = 'S'; // for send
  buffer[i++] = (char) strlen(data); // length of data
  buffer[i++] = '$'; // for $erial data
  memcpy(&buffer[i], (char*) data, strlen(data));
  i+=strlen(data);
  buffer[i] = '\0';
  process_post(PROCESS_BROADCAST, UART2_SEND, &buffer[0]);
PROCESS_END();
}


//uart 2 send
PROCESS(uart2_send, "uart2 send command");
SHELL_COMMAND(uart2_send_command, "u2s", "u2s: send string to uart2", &uart2_send);
/* --------------------------------- */
PROCESS_THREAD(uart2_send, ev, data) {
PROCESS_BEGIN();
  printf("will be printing to uart2\r\n");
  process_post(PROCESS_BROADCAST, UART2_SEND, data);
PROCESS_END();
}

// change rime address (only works for 2byte long addr atm, 1 character long each (0-9))
PROCESS(rimeaddr_change_process, "Change rime address proc");
SHELL_COMMAND(rimeaddr_change, "cra", "cra: change rime address", &rimeaddr_change_process);
/* --------------------------------- */
PROCESS_THREAD(rimeaddr_change_process, ev, data) {
PROCESS_BEGIN();
  unsigned char* ptr = data;
  //set node address
  rimeaddr_t my_addr;
  my_addr.u8[0] = (*ptr++) - 48;
  my_addr.u8[1] = (*ptr) - 48;
  rimeaddr_set_node_addr(&my_addr);
  printf("Rime address changed to %d.%d\n\r", my_addr.u8[0], my_addr.u8[1]);
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
  printf("Collect process started\n\r");
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

// start mesh process
PROCESS(mesh_start_process, "Start mesh process");
SHELL_COMMAND(mesh_start, "mshs", "mshs: start mesh process", &mesh_start_process);
/* --------------------------------- */
PROCESS_THREAD(mesh_start_process, ev, data) {
PROCESS_BEGIN();
  mesh_init();
  printf("Mesh process started\n\r");
PROCESS_END();
}

// end mesh process
PROCESS(mesh_end_process, "End mesh process");
SHELL_COMMAND(mesh_end, "mshe", "mshe: end mesh process", &mesh_end_process);
/* --------------------------------- */
PROCESS_THREAD(mesh_end_process, ev, data) {
PROCESS_BEGIN();
  mesh_exit();
  printf("Mesh process terminated\n\r");
PROCESS_END();
}

// mesh send
PROCESS(mesh_send_process, "Mesh send process");
SHELL_COMMAND(mesh_send_cmd, "mshsnd", "mshsnd <addr> <data>: send via mesh to [addr]", &mesh_send_process);
/* --------------------------------- */
PROCESS_THREAD(mesh_send_process, ev, data) {
PROCESS_BEGIN();
  static int data_len;
  char* str = data;
  char* next;
  if(str == NULL)
    PROCESS_EXIT();

  next = strchr(str, ' ');
  if (next++ == NULL)
    PROCESS_EXIT();

  rimeaddr_t addr;
  addr.u8[0] = *str++ - 48;
  addr.u8[1] = *str - 48;
  mesh_set_dest(addr);

  data_len = strlen(next);

  if (data_len <= 20)
    process_post(&mesh_process, mesh_send_event, next);
  else  // this doesnt really work well, figure out why?
  {
    printf("data too long for a single packet. len: %d; splitting data...\n\r", data_len);
    static char* ptr;
    ptr = next;
    while(data_len > 20)
    {
        char buf[21];
        strncpy(buf, ptr, 20);
        buf[20] = '\0';
        data_len -= 20;
        ptr += 20;
        // need to add \0 at the end of buf?
        process_post(&mesh_process, mesh_send_event, buf);
        PROCESS_WAIT_EVENT_UNTIL(ev == mesh_sent);       
        // need to pause the proceess here?
    }
    process_post(&mesh_process, mesh_send_event, ptr);
  }
  printf("Mesh send requested.\n\r");
PROCESS_END();
}

// start rudolph1 process
PROCESS(rudolph1_start_process, "Start rudolph1 process");
SHELL_COMMAND(rudolph1_start, "r1s", "r1s: start rudolph1 process", &rudolph1_start_process);
/* --------------------------------- */
PROCESS_THREAD(rudolph1_start_process, ev, data) {
PROCESS_BEGIN();
  rudolph1_init();
  printf("rudolph1 process started\n\r");
PROCESS_END();
}

// end rudolph1 process
PROCESS(rudolph1_end_process, "End rudolph1 process");
SHELL_COMMAND(rudolph1_end, "r1e", "r1e: end rudolph1 process", &rudolph1_end_process);
/* --------------------------------- */
PROCESS_THREAD(rudolph1_end_process, ev, data) {
PROCESS_BEGIN();
  rudolph1_exit();
  printf("rudolph1 process terminated\n\r");
PROCESS_END();
}

//rudolph1 send process
PROCESS(rudolph1_send_process, "rudolph1 send process");
SHELL_COMMAND(rudolph1_send_cmd, "r1snd", "r1snd <addr> <filename>: send <filename> via rudolph1 to <addr>", &rudolph1_send_process);
/* --------------------------------- */
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
PROCESS(rudolph1_set_file_process, "rudolph1 set file process");
SHELL_COMMAND(rudolph1_set_file, "r1sf", "r1sf: set rudolph1 file", &rudolph1_set_file_process);
/* --------------------------------- */
PROCESS_THREAD(rudolph1_set_file_process, ev, data) {
PROCESS_BEGIN();
  rudolph1_set_filename(data);
  printf("rudolph1 will save to: %s\n\r", data);
PROCESS_END();
}

// enable UART1 shell input
PROCESS(uart1_shell_enable_process, "UART1 shell enable process");
SHELL_COMMAND(uart1_shell_enable, "u1se", "u1se: enable uart1 shell input", &uart1_shell_enable_process);
/* --------------------------------- */
PROCESS_THREAD(uart1_shell_enable_process, ev, data) {
PROCESS_BEGIN();
  uart1_set_input(serial_line_input_byte);
  printf("uart1 shell input enabled\n\r");
PROCESS_END();
}

// disable UART1 shell input
PROCESS(uart1_shell_disable_process, "UART1 shell disable process");
SHELL_COMMAND(uart1_shell_disable, "u1sd", "u1sd: disable uart1 shell input", &uart1_shell_disable_process);
/* --------------------------------- */
PROCESS_THREAD(uart1_shell_disable_process, ev, data) {
PROCESS_BEGIN();
  uart1_set_input(NULL);
  printf("uart1 shell input disabled\n\r");
PROCESS_END();
}

// enable UART2 shell input
PROCESS(uart2_shell_enable_process, "UART2 shell enable process");
SHELL_COMMAND(uart2_shell_enable, "u2se", "u2se: enable uart2 shell input", &uart2_shell_enable_process);
/* --------------------------------- */
PROCESS_THREAD(uart2_shell_enable_process, ev, data) {
PROCESS_BEGIN();
  uart2_serial_input_init();
PROCESS_END();
}

// disable UART2 shell input
PROCESS(uart2_shell_disable_process, "UART2 shell disable process");
SHELL_COMMAND(uart2_shell_disable, "u2sd", "u2sd: disable uart2 shell input", &uart2_shell_disable_process);
/* --------------------------------- */
PROCESS_THREAD(uart2_shell_disable_process, ev, data) {
PROCESS_BEGIN();
  uart2_serial_input_exit();
PROCESS_END();
}

// enable uart2 sdcard logging TODO: figure out why cant create new file?
PROCESS(uart2_log_enable_process, "UART2 sdcard logging process");
SHELL_COMMAND(uart2_log_enable, "u2le", "u2le: enable uart2 sdcard logging", &uart2_log_enable_process);
/* --------------------------------- */
PROCESS_THREAD(uart2_log_enable_process, ev, data)
{
PROCESS_BEGIN();
  uart2_log_init();
PROCESS_END();
}

// disable uart2 sdcard logging
PROCESS(uart2_log_disable_process, "UART2 end sdcard logging process");
SHELL_COMMAND(uart2_log_disable, "u2ld", "u2ld: disable uart2 sdcard logging", &uart2_log_disable_process);
/* --------------------------------- */
PROCESS_THREAD(uart2_log_disable_process, ev, data)
{
PROCESS_BEGIN();
  uart2_log_exit();
PROCESS_END();
}

void
commands_init()
{
  // contiki commands
  shell_reboot_init();

  // own commands
  shell_register_command(&test_command);
  shell_register_command(&led_command);
  shell_register_command(&lcd_command);
  shell_register_command(&ble_command);
  shell_register_command(&uart2_send_command);
  shell_register_command(&rimeaddr_change);
//  shell_register_command(&collect_start);
//  shell_register_command(&collect_end);
//  shell_register_command(&multihop_start);
//  shell_register_command(&multihop_end);
//  shell_register_command(&multihop_send_cmd);
  shell_register_command(&mesh_start);
  shell_register_command(&mesh_end);
  shell_register_command(&mesh_send_cmd);
  shell_register_command(&rudolph1_start);
  shell_register_command(&rudolph1_end);
  shell_register_command(&rudolph1_send_cmd);
  shell_register_command(&rudolph1_set_file);
  shell_register_command(&uart1_shell_enable);
  shell_register_command(&uart1_shell_disable);
  shell_register_command(&uart2_shell_enable);
  shell_register_command(&uart2_shell_disable);
  shell_register_command(&uart2_log_enable);
  shell_register_command(&uart2_log_disable);
  
}

