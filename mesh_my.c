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
 * $Id: example-mesh.c,v 1.7 2010/01/15 10:24:35 nifi Exp $
 */

/**
 * \file
 *         An example of how the Mesh primitive can be used.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime.h"
#include "net/rime/mesh.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"

#include <stdio.h>
#include <string.h>

#define CHANNEL 132

#define FLASH_LED(l) {leds_on(l); clock_delay_msec(50); leds_off(l); clock_delay_msec(50);}

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static struct mesh_conn mesh;
process_event_t mesh_send_event;
process_event_t mesh_sent;
static rimeaddr_t addr;
/*---------------------------------------------------------------------------*/
PROCESS(mesh_process, "Mesh process");
/*---------------------------------------------------------------------------*/
static void
sent(struct mesh_conn *c)
{
  PRINTF("mesh: packet sent\n");
  process_post(PROCESS_BROADCAST, mesh_sent, NULL);
}

static void
timedout(struct mesh_conn *c)
{
  PRINTF("mesh: packet timedout\n");
}

static void
recv(struct mesh_conn *c, const rimeaddr_t *from, uint8_t hops)
{
  FLASH_LED(LEDS_BLUE);

  PRINTF("Data received from %d.%d: %.*s (%d)\n",
	 from->u8[0], from->u8[1],
	 packetbuf_datalen(), (char *)packetbuf_dataptr(), packetbuf_datalen());

  char* p = packetbuf_dataptr();
  if (strstr(p, "post") != NULL)
  {
    serial_line_input_byte('\n', 0);    // input to serial line 1 (same as uart1)
  }
  else
  {
    char i; // is that smaller in 32bit uC?
    for (i = 0; i < packetbuf_datalen(); i++)
      serial_line_input_byte(*p++, 0);
  }
}

const static struct mesh_callbacks callbacks = {recv, sent, timedout};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mesh_process, ev, data)
{
  PROCESS_EXITHANDLER(mesh_close(&mesh);)
  PROCESS_BEGIN();

  mesh_send_event = process_alloc_event();
  mesh_sent = process_alloc_event();

  mesh_open(&mesh, CHANNEL, &callbacks);

  while(1) {
    

    /* Wait until we get a request from comand line */
    PROCESS_WAIT_EVENT_UNTIL( ev == mesh_send_event);

    if(data != NULL)
    {
      packetbuf_copyfrom(data, strlen(data));
      PRINTF("Sending mesh to %u.%u: %s\n\r", addr.u8[0], addr.u8[1], data);
      /* Send the packet. */
      mesh_send(&mesh, &addr);
      process_post(PROCESS_BROADCAST, mesh_sent, NULL);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

void mesh_init()
{
  process_start(&mesh_process, NULL);
  //printf("Mesh process started\n\r");
}

void mesh_exit()
{
  process_exit(&mesh_process);
  //printf("Mesh process terminated\n\r");
}

void mesh_set_dest(rimeaddr_t dest)
{
  addr = dest;
}
