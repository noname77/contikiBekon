/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * $Id: example-rudolph1.c,v 1.7 2010/01/15 10:24:37 nifi Exp $
 */

/**
 * \file
 *         Testing the rudolph1 code in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime/rudolph1.h"
#include "button-sensors.h"
#include "dev/leds.h"
#include "cfs/cfs.h"
#include <stdio.h>

#define FILESIZE 256
#define RUDOLPH1_CHANNEL 140

#define FLASH_LED(l) {leds_on(l); clock_delay_msec(50); leds_off(l); clock_delay_msec(50);}

#define PRINTADDR(addr) \
  { int __addri;                                            \
    for (__addri = sizeof(addr)-1; __addri > 0; __addri--)  \
      printf("%02X:", addr[__addri]);                       \
    printf("%02X", addr[__addri]);                          \
  }

/*---------------------------------------------------------------------------*/
PROCESS(rudolph1_process, "Rudolph1 process");
/*---------------------------------------------------------------------------*/
static char* file;
static char* file_r;
static char file_to_send[FILESIZE];
static int receive;

static void
write_chunk(struct rudolph1_conn *c, int offset, int flag, uint8_t *data, int datalen)
{
  int fd;

  if(*file == 0 || receive == 0)
    return;

  if (flag == RUDOLPH1_FLAG_NEWFILE) {
    printf("rudolph1 new file incoming at %lu\n", clock_time());
    fd = cfs_open(file, CFS_WRITE);
    
    if (fd < 0)
    {
      printf("Failed to open %s\n", file);
      return;
    }
  } else {
    fd = cfs_open(file, CFS_APPEND);
  }
  
  if (datalen > 0) {
    int ret;
    cfs_seek(fd, offset, CFS_SEEK_SET);
    ret = cfs_write(fd, data, datalen);
		PRINTADDR(rimeaddr_node_addr.u8);
		printf(": write_chunk %d bytes at %d\n", ret, offset);
		FLASH_LED(LEDS_BLUE);
  }

  cfs_close(fd);

  if (flag == RUDOLPH1_FLAG_LASTCHUNK) {
    int i;
		PRINTADDR(rimeaddr_node_addr.u8);
    printf(": rudolph1 entire file received\n");
    FLASH_LED(LEDS_YELLOW);
		FLASH_LED(LEDS_YELLOW);
    // let gateway/sender know
    receive = 0;
    file = "";
  }
}

static int
read_chunk(struct rudolph1_conn *c, int offset, uint8_t *to, int maxsize)
{
  int fd;
  int ret = 0;
 
  if ((offset+maxsize) > FILESIZE)
	  maxsize = (FILESIZE - (offset+maxsize));

  if (maxsize > 0) {
		fd = cfs_open(file_r, CFS_READ);
    if (fd < 0)
    {
      printf("Failed to open %s\n", file_r);
      return;
    }
		cfs_seek(fd, offset, CFS_SEEK_SET);
		ret = cfs_read(fd, to, maxsize);
    //int i;
    //for (i=offset; i<(offset+maxsize); i++)
    //{
    //  to = &file_to_send[i];
    //  to++;
    //}
    //ret=i;  // -1 ?

		PRINTADDR(rimeaddr_node_addr.u8);
		printf(": read_chunk %d bytes at %d, %d\n", ret, offset, (unsigned char)to[0]);
		FLASH_LED(LEDS_GREEN);

		cfs_close(fd);
  }
  else {
		PRINTADDR(rimeaddr_node_addr.u8);
    file_r = "";
	  printf(": nothing else to send\n");
  }

  return ret;
}


const static struct rudolph1_callbacks rudolph1_call = {write_chunk, read_chunk};
static struct rudolph1_conn rudolph1;
process_event_t rudolph1_send_event;
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(rudolph1_process, ev, data)
{
  static int fd;
  int i;
  PROCESS_EXITHANDLER(rudolph1_close(&rudolph1);)
  PROCESS_BEGIN();

  rudolph1_send_event = process_alloc_event();

  PROCESS_PAUSE();
  
  file = "";
  file_r = "";

	uSDcard_init(); //just in case
	printf("uSD card initialised.\n");

  rudolph1_open(&rudolph1, RUDOLPH1_CHANNEL, &rudolph1_call);
  //SENSORS_ACTIVATE(button_sensor);

//change to an array
/*
  fd = cfs_open("hej.txt", CFS_WRITE);
  for(i = 0; i < FILESIZE; i++) {
		unsigned char buf = 'A' + i%26;
		cfs_write(fd, &buf, 1);
    //file_to_send[i] = buf;
  }
  cfs_close(fd);

  set_power(0x01);
*/
  while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == rudolph1_send_event);
    file_r = data;
		printf("Sending rudolph1 file: %s\n\r", data);
    // send rudolph chunk every 2 seconds
		rudolph1_send(&rudolph1, CLOCK_SECOND * 2);

/*
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
	  printf("Stopping\n");
    rudolph1_stop(&rudolph1);
*/
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

void
rudolph1_init()
{
  process_start(&rudolph1_process, NULL);
}

void
rudolph1_exit()
{
  process_exit(&rudolph1_process);
}

void
rudolph1_pause()
{
  rudolph1_stop(&rudolph1);  
}

void
rudolph1_set_receive(int rec)
{
  receive = rec;
}

void
rudolph1_set_filename(char* f)
{
  file = f;
  receive = 1;
}

void
rudolph1_read_from(char* f)
{
  file_r = f;
  //printf("rudolph1 will read from: %s\n\r", file_r);
}
