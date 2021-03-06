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
 * $Id: example-collect.c,v 1.16 2011/01/10 15:11:44 adamdunkels Exp $
 */

/**
 * \file
 *         Example of how the collect primitive works.
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "lib/random.h"
#include "net/rime.h"
#include "net/rime/collect.h"
#include "dev/leds.h"
#include "dev/button-sensor.h"

#include "net/netstack.h"

#include <stdio.h>

static struct collect_conn tc;

#define CHANNEL 130


/*---------------------------------------------------------------------------*/
PROCESS(collect_process, "Collect process");
/*---------------------------------------------------------------------------*/
static void
recv(const rimeaddr_t *originator, uint8_t seqno, uint8_t hops)
{
  printf("Sink got message from %d.%d, seqno %d, hops %d: len %d '%s'\n",
	 originator->u8[0], originator->u8[1],
	 seqno, hops,
	 packetbuf_datalen(),
	 (char *)packetbuf_dataptr());
}
/*---------------------------------------------------------------------------*/
static const struct collect_callbacks callbacks = { recv };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(collect_process, ev, data)
{
  static struct etimer periodic;
  static struct etimer et;
  
  PROCESS_EXITHANDLER(collect_close(&tc); etimer_stop(&periodic); etimer_stop(&et); printf("ended");)
  PROCESS_BEGIN();

  collect_open(&tc, CHANNEL, COLLECT_ROUTER, &callbacks);

  if(rimeaddr_node_addr.u8[0] == 1 &&
     rimeaddr_node_addr.u8[1] == 0) {
	printf("I am sink\n");
	collect_set_sink(&tc, 1);
  }

  /* Allow some time for the network to settle. */
  etimer_set(&et, 5 * CLOCK_SECOND);
  PROCESS_WAIT_UNTIL(etimer_expired(&et));

  while(1) {
    /* Send a packet every 15 + max2 seconds. */
    if(etimer_expired(&periodic)) {
      etimer_set(&periodic, CLOCK_SECOND * 15);
      etimer_set(&et, random_rand() % (CLOCK_SECOND * 2));
    }
    // change that to a proper implementation
    PROCESS_WAIT_EVENT();

    if(etimer_expired(&et) && !(rimeaddr_node_addr.u8[0] == 1 &&
     rimeaddr_node_addr.u8[1] == 0)) {
      static rimeaddr_t oldparent;
      const rimeaddr_t *parent;

      printf("Sending\n");
      packetbuf_clear();
      packetbuf_set_datalen(sprintf(packetbuf_dataptr(),
				  "%s", "Hello") + 1);
      collect_send(&tc, 15);

      // what is this?
      parent = collect_parent(&tc);
      if(!rimeaddr_cmp(parent, &oldparent)) {
        if(!rimeaddr_cmp(&oldparent, &rimeaddr_null)) {
          printf("#L %d 0\n", oldparent.u8[0]);
        }
        if(!rimeaddr_cmp(parent, &rimeaddr_null)) {
          printf("#L %d 1\n", parent->u8[0]);
        }
        rimeaddr_copy(&oldparent, parent);
      }
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

void collect_init()
{
  process_start(&collect_process, NULL);
}

void collect_exit()
{
  process_exit(&collect_process);
}
