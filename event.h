/*
 * Headers for the event handling code.
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
 * $Id: event.h,v 1.2 1999/05/19 08:22:16 kefka Exp $
 */

#ifndef EVENT_H
#define EVENT_H

#include "status.h"
#include "conf.h"

/* Prototypes. */
void event_action(Event *event);
void event_macrocall(Macro *macro);
void event_execute_commands(Macro_Commands *macro_commands);

#endif
