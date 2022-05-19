/*
 * Handle reading and interpreting data from the x10.
 * Copyright (C) 1999  Steven Brown
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * Steven Brown <swbrown@ucsd.edu>
 *
 * $Id: read.c,v 1.5 2000/02/06 01:25:42 swbrown Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "x10.h"
#include "error.h"
#include "read.h"
#include "monitor.h"
#include "ppowerd.h"
#include "ppowerd-proto.h"
#include "conf-proto.h"
#include "status.h"
#include "event.h"

/*
 * Address buffer.  The cm11a sometimes sends buffers that have trailing
 * addresses but no functions.  These functions appear in the next buffer. 
 * To deal with this, we keep a buffer of addresses.  I'm figuring that
 * since you can only multiple-address things in the same housecode that the
 * max size should be 16 addresses.
 */
static unsigned char address_buffer[16];
static int address_buffer_count=0;
static unsigned char address_buffer_housecode;


/* Handle reading and handling requests from the x10. */
void read_x10(int fd) {
	unsigned char command;
	char buffer[16];
	
	/* Read the byte sent by the x10 hardware. */
	if(x10_read(fd, &command, 1) != 1) {
		return;
	}
	
	/* Is this a data poll? */
	if(command == 0x5a) {
		read_x10_poll(fd);
	}
	
	/* Is this a power-fail time request poll? */
	else if(command == 0xa5) {
		
		/* Build a time response to send to the hardware. */
		buffer[0]=(char) 0x9b;
		x10_build_time(&buffer[1], time(NULL), -1, TIME_TIMER_PURGE);
		
		/* Send this response to the hardware. */
		if(x10_write_message(fd, &buffer, 7) != 0) {
			
			/* 
			 * This shouldn't fail, the cm11a blocks in this
			 * mode until it is answered.  The only way it
			 * should fail is if we are in this mode due to
			 * static and a poll came in to block us.
			 */
			return;
		}
	}
	
	/* It was an unknown command (probably static or leftovers). */
	else {
		return;
	}
	
	return;
}


/* 
 * Handles a data poll request from the x10. 
 *
 * *** 
 * Need to do better checking that the event is valid.  Make sure things
 * that need at least one device are getting them.  Sometimes after not
 * being listened to for a while, the cm11a will send us a buffer with a
 * function like 'ON' in it but no addresses.
 */
void read_x10_poll(int fd) {
	unsigned char x10_buffer[8];
	unsigned char command;
	unsigned char buffer_size;
	unsigned char function_byte;
	char string_buffer[256];
	int i,j;
	Event event;
	
	/* Acknowledge the x10's poll. */
	command=0xc3;
	if(x10_write(fd, &command, 1) != 1) {
		return;
	}
	
	/* Get the size of the request. */
	if(x10_read(fd, &buffer_size, 1) != 1) {
		
		/* 
		 * Errors here are unexpected since if we didn't get the ack
		 * through, we should at least read another 'poll' byte
		 * here.
		 */
		return;
	}
	
	/* Must have at least 2 bytes or it's just weird. */
	if(buffer_size < 2) {
		return;
	}
	
	/* Read in the function byte. */
	if(x10_read(fd, &function_byte, 1) != 1) {
		return;
	}
	
	/* Read in the buffer from the x10. */
	if(x10_read(fd, &x10_buffer, buffer_size - 1) != buffer_size - 1) {
		return;
	}
	
	/* Print packet info to debug. */
	sprintf(string_buffer, "X10 packet> size: %i, function %02x, data:", buffer_size, function_byte);
	for(i=0; i < buffer_size - 1; i++) {
		sprintf(string_buffer + strlen(string_buffer), " %02x", x10_buffer[i]);
	}
	
}
