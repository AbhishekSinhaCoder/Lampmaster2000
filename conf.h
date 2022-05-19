/*
 * Headers for the configuration file handler.
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
 * $Id: conf.h,v 1.8 1999/05/27 06:31:14 kefka Exp $
 */

#ifndef CONF_H
#define CONF_H

#include "stacklist.h"
#include "table.h"

/* Typedefs. */
typedef struct device Device;
typedef struct alias Alias;
typedef struct function Function;
typedef struct state State;
typedef struct macro Macro;
typedef struct macro_command Macro_Command;
typedef struct macro_commands Macro_Commands;
typedef struct macro_command_run Macro_Command_Run;
typedef struct macro_command_if Macro_Command_If;
typedef struct macro_command_command Macro_Command_Command;
typedef struct macro_command_data Macro_Command_Data;
typedef struct expression Expression;
typedef struct macro_command_else Macro_Command_Else;

/* Prototypes. */
void conf_parse(char *filename);
void conf_free(void);
void conf_alias_insert_device(Alias *alias, Device *device);
void conf_devicelist_free(Stacklist *devicelist);
void conf_macrolist_free(Stacklist *macrolist);
void conf_aliaslist_free(Stacklist *aliaslist);
Device *conf_device_create(char *name, unsigned char homecode, unsigned char devicecode, int type);
Alias *conf_alias_create(char *name, int devices, ...);
Alias *conf_aliaslist_lookup(char *name);
void conf_alias_sort(Alias *alias);
void conf_aliaslist_sort(Stacklist *aliaslist);

/* Structure for expressions (used with IF). */
struct expression {
	/* *** */
	int bookmark;
};

/* Structure for devices. */
struct device {
	
	/* The name of this device. */
	char *name;
	
	/* The housecode:devicecode of this device. */
	unsigned char housecode;
	unsigned char devicecode;
	
	/* The type of the device like DEVICE_LAMP. */
	int type;
	
	/* If the status of this device is known, we set this flag to true. */
	int status_known;
	
	/* 
	 * Status of this device if known.  Note that this isn't guaranteed
	 * to be correct.  It is very easy to confuse the cm11a.
	 */
	union {
		
		/* For appliances and similar.  1 for ON, 0 for OFF. */
		int active;
		
		/* For lamps.  0-22 are valid settings.  22 when ON. */
		int level;
	} status;
};

/* Structure for aliases. */
struct alias {
	
	/* The name of the alias. */
	char *name;
	
	/* Number of devices and the device pointer table. */
	int devices;
	Device **device;
};

/* Function values. */
struct function {

	/* The command, like 'ON'. */
	unsigned char command;
	
	/* 
	 * First extended byte.  Used for extended transmissions and for
	 * bright/dim.
	 */
	unsigned char extended1;
	 
	/* Second extended byte.  Used for extended transmissions. */
	unsigned char extended2;
};

/* Structure for states. */
struct state {
	
	/* Function of the state. */
	Function function;
	
	/* List of devices for this state. */
	int devices;
	Device **device;
};

/* Structure for macro commands, holds a list of commands. */
struct macro_commands {
	
	/* Number of commands we are holding. */
	int commands;
	
	/* Table of command structures. */
	Macro_Command *macro_command;
};

/* ELSE command's macro data. */
struct macro_command_else {
	
	/* Commands to be executed. */
	Macro_Commands macro_commands;
};

/* IF command's macro data. */
struct macro_command_if {
	
	/* Expression to execute the commands on. */
	Expression expression;
	
	/* Commands to be executed. */
	Macro_Commands macro_commands;
};

/* COMMAND command's macro data. */
struct macro_command_command {
	
	/* State to change to. */
	State state;
};

/* RUN command's macro data. */
struct macro_command_run {
	
	/* Commandline to run. */
	char *commandline;
};

/* Types for macro commands. */
#define MACRO_COMMAND_IF 0
#define MACRO_COMMAND_COMMAND 1
#define MACRO_COMMAND_RUN 2
#define MACRO_COMMAND_ELSE 3

/* Structure for an individual macro command.  Used with macros. */
struct macro_command {
	
	/* The type of the command. */
	int type;
	
	/* Info for the different types of commands. */
	union {
		Macro_Command_If command_if;
		Macro_Command_Else command_else;
		Macro_Command_Command command_command;
		Macro_Command_Run command_run;
	} info;
};

/* Structure for macros. */
struct macro {
	
	/* Optional name, NULL if it doesn't exist. */
	char *name;
	
	/* States the macro is run on. */
	Table *statetable;
	
	/* Commands to be evaluated. */
	Macro_Commands macro_commands;
};

#endif
