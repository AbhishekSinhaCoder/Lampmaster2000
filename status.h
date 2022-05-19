/*
 * Headers for the status handling code.  Deals with events and such.
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
 * $Id: status.h,v 1.3 1999/05/19 08:22:17 kefka Exp $
 */

#ifndef STATUS_H
#define STATUS_H

#include <time.h>

/* Typedefs. */
typedef struct event Event;

/* Prototypes. */
void status_display_event(Event *event);

/* Event structure to report happenings to monitoring clients. */
struct event {
	unsigned char command;
	unsigned char housecode;
	int devices;
	unsigned char device[16];
	time_t time;
	unsigned char extended1;
	unsigned char extended2;
};

/* 
 * ***
 * This will be the struct that the daemon sends when a status changes.  It
 * will send a bunch of these when the monitor first connects to inform it
 * of what's going on, then one along with each event to tell it what the
 * event changed.  Or, alternatively, have a function for 
 */
struct status {
	int dummy;	
};

#endif
