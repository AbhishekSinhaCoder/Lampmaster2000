/*
 * Prototypes for external daemon variables.
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
 * $Id: ppowerd-proto.h,v 1.2 1999/05/19 08:22:16 kefka Exp $
 */

#ifndef PPOWERD_PROTO_H
#define PPOWERD_PROTO_H

#include <sys/poll.h>
#include "config.h"
#include "ppowerd.h"

/* Externals available for ppowerd parts. */
extern char *progname;
extern struct pollfd pollfd[POLL_MAX_COUNT];
extern int user_monitor_sockets;
extern int config_housecode;

#endif
