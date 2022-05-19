/*
 * Daemon headers and prototypes.
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
 * $Id: ppowerd.h,v 1.4 1999/05/19 08:22:16 kefka Exp $
 */

#ifndef PPOWERD_H
#define PPOWERD_H

#include <time.h>

/* Typedefs. */
typedef struct client_command Client_Command;
typedef struct device_status Device_Status;

/* 
 * Positions in the poll structure for our fd's.  User monitor must be last
 * as it is the only variable sized field.
 */
#define POLL_X10 0
#define POLL_DAEMON 1
#define POLL_DAEMON_MONITOR 2
#define POLL_USER_MONITOR 3

/* Sizes for the poll structures. */
#define POLL_DEFAULT_COUNT 3
#define POLL_MAX_COUNT (POLL_DEFAULT_COUNT + DAEMON_MONITOR_MAX)

/* Time to wait for sockets. */
#define USER_READ_TIMEOUT 5000000

/* Requests the client can make. */
#define REQUEST_COMMAND 0

/* Client command structure.  Sent to the daemon on a command socket. */
struct client_command {
	int request;
	unsigned char command;
	unsigned char housecode;
	int value;
	int devices;
	unsigned char device[16];
};

#endif
