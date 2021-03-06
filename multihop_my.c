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
 * $Id: example-multihop.c,v 1.7 2010/01/15 10:24:36 nifi Exp $
 */

/**
 * \file
 *         Testing the multihop forwarding layer (multihop) in Rime
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 *
 *         This example shows how to use the multihop Rime module, how
 *         to use the announcement mechanism, how to manage a list
 *         with the list module, and how to allocate memory with the
 *         memb module.
 *
 *         The multihop module provides hooks for forwarding packets
 *         in a multi-hop fashion, but does not implement any routing
 *         protocol. A routing mechanism must be provided by the
 *         application or protocol running on top of the multihop
 *         module. In this case, this example program provides the
 *         routing mechanism.
 *
 *         The routing mechanism implemented by this example program
 *         is very simple: it forwards every incoming packet to a
 *         random neighbor. The program maintains a list of neighbors,
 *         which it populated through the use of the announcement
 *         mechanism.
 *
 *         The neighbor list is populated by incoming announcements
 *         from neighbors. The program maintains a list of neighbors,
 *         where each entry is allocated from a MEMB() (memory block
 *         pool). Each neighbor has a timeout so that they do not
 *         occupy their list entry for too long.
 *
 *         When a packet arrives to the node, the function forward()
 *         is called by the multihop layer. This function picks a
 *         random neighbor to send the packet to. The packet is
 *         forwarded by every node in the network until it reaches its
 *         final destination (or is discarded in transit due to a
 *         transmission error or a collision).
 *
 */

#include "contiki.h"
#include "net/rime.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "dev/serial-line.h"

#include <stdio.h>
#include <string.h>

#define FLASH_LED(l) {leds_on(l); clock_delay_msec(50); leds_off(l); clock_delay_msec(50);}

#define CHANNEL 136

struct example_neighbor {
  struct example_neighbor *next;
  rimeaddr_t addr;
  struct ctimer ctimer;
};

process_event_t mh_send;

#define NEIGHBOR_TIMEOUT 60 * CLOCK_SECOND
#define MAX_NEIGHBORS 16
LIST(neighbor_table);
MEMB(neighbor_mem, struct example_neighbor, MAX_NEIGHBORS);
/*---------------------------------------------------------------------------*/
PROCESS(multihop_process, "Multihop process");
/*---------------------------------------------------------------------------*/
/*
 * This function is called by the ctimer present in each neighbor
 * table entry. The function removes the neighbor from the table
 * because it has become too old.
 */
static void
remove_neighbor(void *n)
{
  struct example_neighbor *e = n;

  list_remove(neighbor_table, e);
  memb_free(&neighbor_mem, e);
}
/*---------------------------------------------------------------------------*/
/*
 * This function is called when an incoming announcement arrives. The
 * function checks the neighbor table to see if the neighbor is
 * already present in the list. If the neighbor is not present in the
 * list, a new neighbor table entry is allocated and is added to the
 * neighbor table.
 */
static void
received_announcement(struct announcement *a,
                      const rimeaddr_t *from,
		      uint16_t id, uint16_t value)
{
  struct example_neighbor *e;

  /*  printf("Got announcement from %d.%d, id %d, value %d\n",
      from->u8[0], from->u8[1], id, value);*/

  /* We received an announcement from a neighbor so we need to update
     the neighbor list, or add a new entry to the table. */
  for(e = list_head(neighbor_table); e != NULL; e = e->next) {
    if(rimeaddr_cmp(from, &e->addr)) {
      /* Our neighbor was found, so we update the timeout. */
      ctimer_set(&e->ctimer, NEIGHBOR_TIMEOUT, remove_neighbor, e);
      return;
    }
  }

  /* The neighbor was not found in the list, so we add a new entry by
     allocating memory from the neighbor_mem pool, fill in the
     necessary fields, and add it to the list. */
  e = memb_alloc(&neighbor_mem);
  if(e != NULL) {
    rimeaddr_copy(&e->addr, from);
    list_add(neighbor_table, e);
    ctimer_set(&e->ctimer, NEIGHBOR_TIMEOUT, remove_neighbor, e);
  }
}
static struct announcement example_announcement;
/*---------------------------------------------------------------------------*/
/*
 * This function is called at the final recepient of the message.
 */
static void
recv(struct multihop_conn *c, const rimeaddr_t *sender,
     const rimeaddr_t *prevhop,
     uint8_t hops)
{
  FLASH_LED(LEDS_BLUE);
  //printf("multihop message received '%s'\n", (char *)packetbuf_dataptr());
  // put what we received into shell
  
  char* p = packetbuf_dataptr();
  char * tmp = p;
  //if(strcmp(p, "post") == 0)
  // ugly maybe will work
  if ((*tmp++ == 'p') && (*tmp++ == 'o') && (*tmp++ == 's') && (*tmp++ == 't'))
  {
    serial_line_input_byte('\n', 0);    // input to serial line 1 (same as uart1)
    //PROCESS_PAUSE(); // need that?
  }
  else
  {
    char i; // is that smaller in 32bit uC?
    for (i = 0; i < packetbuf_datalen(); i++)
      serial_line_input_byte(*p++, 0);
   }
  
  //shell_input(packetbuf_dataptr(), strlen(packetbuf_dataptr()));
}
/*
 * This function is called to forward a packet. The function picks a
 * random neighbor from the neighbor list and returns its address. The
 * multihop layer sends the packet to this address. If no neighbor is
 * found, the function returns NULL to signal to the multihop layer
 * that the packet should be dropped.
 */
static rimeaddr_t *
forward(struct multihop_conn *c,
	const rimeaddr_t *originator, const rimeaddr_t *dest,
	const rimeaddr_t *prevhop, uint8_t hops)
{
  /* Find a random neighbor to send to. */
  int num, i;
  struct example_neighbor *n;

  if(list_length(neighbor_table) > 0) {
    num = random_rand() % list_length(neighbor_table);
    i = 0;
    for(n = list_head(neighbor_table); n != NULL && i != num; n = n->next) {
      ++i;
    }
    if(n != NULL) {
      printf("%d.%d: Forwarding packet to %d.%d (%d in list), hops %d\n",
	     rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1],
	     n->addr.u8[0], n->addr.u8[1], num,
	     packetbuf_attr(PACKETBUF_ATTR_HOPS));
      return &n->addr;
    }
  }
  printf("%d.%d: did not find a neighbor to foward to\n",
	 rimeaddr_node_addr.u8[0], rimeaddr_node_addr.u8[1]);
  return NULL;
}
static const struct multihop_callbacks multihop_call = {recv, forward};
static struct multihop_conn multihop;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(multihop_process, ev, data)
{
  PROCESS_EXITHANDLER(multihop_close(&multihop);)
    
  PROCESS_BEGIN();

  mh_send = process_alloc_event();
  
  /* Initialize the memory for the neighbor table entries. */
  memb_init(&neighbor_mem);

  /* Initialize the list used for the neighbor table. */
  list_init(neighbor_table);

  /* Open a multihop connection on Rime channel CHANNEL. */
  multihop_open(&multihop, CHANNEL, &multihop_call);

  /* Register an announcement with the same announcement ID as the
     Rime channel we use to open the multihop connection above. */
  announcement_register(&example_announcement,
			CHANNEL,
			received_announcement);

  /* Set a dummy value to start sending out announcments. */
  announcement_set_value(&example_announcement, 0);

  /* Activate the button sensor. We use the button to drive traffic -
     when the button is pressed, a packet is sent. */
  SENSORS_ACTIVATE(button_sensor);

  /* Loop forever, send a packet when the button is pressed. */
  while(1) {
    rimeaddr_t addr;

    /* Wait until we get a sensor event with the button sensor as data. */
    PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event &&
			     data == &button_sensor) || ev == mh_send);

    //on button event send test stuff
    if (ev == sensors_event)
    {
      /* Copy the "Hello" to the packet buffer. */
      packetbuf_copyfrom("Hello multihop", 15);

      /* Set the Rime address of the final receiver of the packet to
         1.0. This is a value that happens to work nicely in a Cooja
         simulation (because the default simulation setup creates one
         node with address 1.0). */

      addr.u8[0] = 1;
      addr.u8[1] = 0;

      /* Send the packet. */
      multihop_send(&multihop, &addr);
      packetbuf_copyfrom("post", 5);
      multihop_send(&multihop, &addr);
    }
    else // request from comand line
    {
      char* ptr = data;
      char* next;
      if(ptr != NULL)
      {
        next = strchr(data, ' ');
        if (next != NULL)
        {
          addr.u8[0] = *ptr++ - 48;
          addr.u8[1] = *ptr - 48;
          packetbuf_copyfrom(++next, strlen(next));
          printf("Sending multihop to %u.%u\n\r", addr.u8[0], addr.u8[1]);
          /* Send the packet. */
          multihop_send(&multihop, &addr);
        }
      }
    }

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

void multihop_init()
{
  process_start(&multihop_process, NULL);
}

void multihop_exit()
{
  process_exit(&multihop_process);
}
