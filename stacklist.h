/*
 * Stacklist handling headers.
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
 * $Id: stacklist.h,v 1.2 1999/05/19 08:22:16 kefka Exp $
 */

#ifndef STACKLIST_H
#define STACKLIST_H

/* Typedefs. */
typedef struct stacklist Stacklist;
typedef struct stacklist_entry Stacklist_Entry;

/* Prototypes. */
void stacklist_remove(Stacklist *stacklist, Stacklist_Entry *stacklist_entry);
void stacklist_init(Stacklist *stacklist);
void stacklist_insert(Stacklist *stacklist, void *info);
void stacklist_free(Stacklist *stacklist);

/* Header structure for a stacklist. */
struct stacklist {
	
	/* The first entry in the list. */
	Stacklist_Entry *first;
	
	/* The number of entries in the list. */
	int entries;
};

/* Entry structure for a stacklist. */
struct stacklist_entry {
	
	/* The info for this entry. */
	void *info;
	
	/* The previous stacklist entry. */
	Stacklist_Entry *previous;
};

#endif
