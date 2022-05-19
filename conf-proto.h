/*
 * External variable prototypes needed for the configuration file handler.
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
 * $Id: conf-proto.h,v 1.4 1999/05/19 08:22:16 kefka Exp $
 */

#ifndef CONF_PROTO_H
#define CONF_PROTO_H

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <stdio.h>
#include <limits.h>
#include "stacklist.h"

/* Configuration options. */
extern char *conf_tty;
extern uid_t conf_daemon_socket_uid;
extern gid_t conf_daemon_socket_gid;
extern int conf_daemon_socket_mode;
extern uid_t conf_daemon_monitor_socket_uid;
extern gid_t conf_daemon_monitor_socket_gid;
extern int conf_daemon_monitor_socket_mode;
extern int conf_housecode;
extern char *conf_daemon_dir;
extern int conf_daemon_command_timeout;

extern Stacklist conf_devicelist;
extern Stacklist conf_aliaslist;
extern Stacklist conf_macrolist;

extern char conf_daemon_monitor_socket_path[PATH_MAX];
extern char conf_daemon_socket_path[PATH_MAX];

/* Just to make it easier on us to translate codes. */
extern unsigned char housecode_table[];
extern unsigned char devicecode_table[];

#endif
