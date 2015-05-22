#include "contiki.h"
#include "mc1322x.h"
#include "ble.h"
#include "contiki-uart.h"
#include "SDcard.h"
#include "io-pins.h"
#include "dev/serial-line.h"
#include "dev/leds.h"

#include <stdio.h>
#include <string.h>
/*
PROCESS(ble_process, "BLE Process");

PROCESS_THREAD(ble_process, ev, data)
{
  PROCESS_BEGIN();

  //handshake
    //send some character and wait for reply
  // ble setup
    // disable uart 2 shell input
    uart2_serial_input_exit();
    // enable direct control over contiki through tilde prefix
    if (data == '~') shell_input(++data, strlen(data));
    // set atributes contents
    // read atribute contents
    // get seen device's MAC address
    // setup direct communication with smartphone
  // sending data

  // receiving data

  PROCESS_END();
}
*/

int ble112_send_command(char* command, char command_len, char* response)
{
  char rsp[6];
  char i;

  for(i=0; i<command_len; i++)
  {
    //printf("0x%02x ", command[i]);
    uart2_putc(command[i]);
  }  
  //wait until we received a response
  while (!uart2_can_get());
  //read the response back
  for(i=0; i<6; i++)
  {
    rsp[i] = uart2_getc();
    //printf("0x%02x ", rsp[i]);
  }
  // check response
  if(memcmp(rsp, response, 6) != 0)
  {
    return -1;
  }
  else return 1;
}

PROCESS(ble_process, "BLE Process");
PROCESS_THREAD(ble_process, ev, data){
PROCESS_BEGIN();
  while(1)
  {
    PROCESS_WAIT_EVENT_UNTIL(ev == serial_line2_event_message);

  }
PROCESS_END();
}


PROCESS(ble_dfu_process, "BLE DFU Process");
PROCESS_THREAD(ble_dfu_process, ev, data){
PROCESS_BEGIN();

/* DFU procedure:
    1. Send dfu_reset(0x01) command: 00 01 09 00 01 (no response back)
    2. Send dfu_flash_set_address(0x1000) command: 00 04 09 01 00 10 00 00 (check for response)
    3. Send dfu_flash_upload([bytes]) command repeatedly (check for response)
        - Packet's data payload length byte is 0x41 (65 bytes)
        - Each uint8array should have a length byte of 0x40 (64 bytes) followed by 64 bytes of firmware data
        - Each response should come back as 00 02 09 02 00 00 if successful
        - First packet starts from the :1010... line of the out.hex file, e.g. line 258
    4. Send dfu_flash_upload_finish() command: 00 00 09 03 (check for response)
    5. Send dfu_reset(0x00) command: 00 01 09 00 00 (no response back)
    6. Module will boot back in normal mode, DFU is complete!
*/

  const char FILENAME[] = "ble112.bin";
  const unsigned int DFU_INITIAL_OFFSET = 4096;  // first 4KB is stuff we cannot change
  const unsigned int DFU_CHUNK_SIZE = 64;
  const unsigned int DFU_ALL_BYTES = 124928; 

  const char dfu_reset[] = {0x00, 0x01, 0x09, 0x00};  // ad 0x01 to reset into dfu or 0x00 to reset normally
  const char dfu_flash_set_address[] = {0x00, 0x04, 0x09, 0x01, 0x00, 0x10, 0x00, 0x00};  // set address to 0x1000
  const char dfu_flash_upload[] = {0x00, 0x41, 0x09, 0x02, 0x40}; // followed by 64 bytes of actual data
  const char dfu_flash_upload_finish[] = {0x00, 0x00, 0x09, 0x03};
  // TODO: maybe do it better
  const char rsp_dfu_flash_set_address[] = {0x00, 0x02, 0x09, 0x01, 0x00, 0x00};
  const char rsp_dfu_flash_upload[] = {0x00, 0x02, 0x09, 0x02, 0x00, 0x00};
  const char rsp_dfu_flash_upload_finish[] = {0x00, 0x02, 0x09, 0x03, 0x00, 0x00};

  unsigned int sent;
  static char buf[DFU_CHUNK_SIZE + 5];
  char i;

  // send dfu reset command
  for(i=0; i<4; i++)
    uart2_putc(dfu_reset[i]);
  uart2_putc(0x01);
  //should be in dfu now
  printf("dfu reset command sent.\n\r");
  //wait a sec to make sure ble booted
  clock_delay_msec(500);   //TODO: figure out smallest reliable

  //set flash address
  if(ble112_send_command((char*) dfu_flash_set_address, 8, (char*) rsp_dfu_flash_set_address) != 1)
  {
    // something went wrong.. print debug and terminate
    printf("ble112 dfu error: flash set address rsp != 0\n\r");
    printf("Update failed. Try again.\n\r");
    PROCESS_EXIT();
  }
  else
    printf("flash address set command sent.\n\r");

  sent = 0;
  while (sent != DFU_ALL_BYTES)
  {
    // fill the buffer
    memcpy(buf, dfu_flash_upload, 5); // copy bgapi flash upload command to the buffer
    file_read((char*) FILENAME, &buf[5], (unsigned int) DFU_CHUNK_SIZE, (unsigned int) (DFU_INITIAL_OFFSET + sent));
    
    //send the command and data
    if(ble112_send_command(buf, DFU_CHUNK_SIZE + 5, (char*) rsp_dfu_flash_upload) != 1)
    {
      // something went wrong.. print debug and terminate
      printf("ble112 dfu error: flash upload rsp != 0\n\r");
      printf("Update failed. Try again.\n\r");
      PROCESS_EXIT();
    }
    
    sent += DFU_CHUNK_SIZE;
    printf("Sent: %u\n\r", sent);
  }
  // all update data sent here
  printf("all update data chunks sent.\n\r");
  // send flash upload finish command  
  if(ble112_send_command((char*) dfu_flash_upload_finish, 4, (char*) rsp_dfu_flash_upload_finish) != 1)
  {
    // something went wrong.. print debug and terminate
    printf("ble112 dfu error: flash upload finish rsp != 0\n\r");
    printf("Update failed. Try again.\n\r");
    PROCESS_EXIT();
  }
  else
    printf("flash upload finish command sent.\n\r");
  // send dfu reset command
  for(i=0; i<4; i++)
    uart2_putc(dfu_reset[i]);
  uart2_putc(0x00);
    
  printf("Update success!\n\r");
PROCESS_END();
}

void ble_reset()
{
  ioPins_setValue(BLE_RST_PIN, 0);  // ble reset low
  clock_delay_msec(10);
  ioPins_setValue(BLE_RST_PIN, 1);  // ble reset high
}

void ble_dfu_init()
{
  process_start(&ble_dfu_process, NULL);
}

void ble_init()
{
  process_start(&ble_process, NULL);
}
