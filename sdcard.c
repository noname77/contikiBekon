/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Example on how to use CFS/Coffee.
 * \author
 *         Nicolas Tsiftes <nvt@sics.se>
 */

#include <stdio.h>
#include <string.h>

#include "contiki.h"
#include "cfs/cfs.h"
#include "cfs/cfs-coffee.h"
#include "ff.h"


PROCESS(example_coffee_process, "Coffee example");
AUTOSTART_PROCESSES(&example_coffee_process);

#define FILENAME "test.txt"

PROCESS_THREAD(example_coffee_process, ev, data)
{
  char *message     = "Hello world\n";
  int   messageSize = strlen(message);

  PROCESS_BEGIN();

  // Initialise the micro SD card.
  // We must have this statement before using the card
  //
	uSDcard_init();
  printf("Initialised uSD card\n");

  // If our test file exists, remove it
  //
  cfs_remove(FILENAME);

  // Obtain a file descriptor for the file, capable of
  // handling both reads and writes.
  //
  int fd = cfs_open(FILENAME, CFS_WRITE | CFS_APPEND | CFS_READ);
  if (fd < 0) {
    printf("Failed to open %s\n", FILENAME);
    return 0;
  }

  printf("Opened file %s\n", FILENAME);

  // Write a message to the file
  //
  int r = cfs_write(fd, message, messageSize);
  if (r < messageSize) {
    printf("Failed to write %d bytes to %s\n", messageSize, FILENAME);
    cfs_close(fd);
    return 0;
  }

  printf("Wrote message: %s", message);

  // Before reading back the message, we need to move
  // the file pointer to the beginning of the file
  //
  if (cfs_seek(fd, 0, CFS_SEEK_SET) != 0) {
    printf("Seek failed\n");
    cfs_close(fd);
    return 0;
  }

  printf("Seek to position 0 completed\n");

  // Read the message back
  //
  char message_in[16];
  r = cfs_read(fd, message_in, messageSize);

  if (r < messageSize) {
    printf("Failed to read %d bytes from %s, got %d bytes\n",
             messageSize, FILENAME, r);
      cfs_close(fd);
      return 0;
  }
	message_in[r] = '\0';

 	printf("Read message: %s", message_in);

  // Close the file and release resources associated with fd
  //
  cfs_close(fd);

	// Now power down the micro SD card
  //
	uSDcard_power_down();
 	printf("uSD card powered down\n");

  PROCESS_END();
}
