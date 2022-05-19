/*
 * Handle the cm11a interface to the x10 hardware.
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
 * $Id: x10.c,v 1.9 2000/02/06 01:37:08 swbrown Exp $
 */

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
//#include "error.h"
#include "x10.h"
#include "read.h"


/* 
 * Open the x10 device. 
 *
 * Description of how to do the serial handling came from some mini serial
 * port programming howto.
 */
int	x10_open(char *x10_tty_name) {
	struct termios termios;
	int	fd;
	
	/* Allocate us a structure for the x10 info. */
	/* 
	 * Open the x10 tty device.
	 */
	fd=open(x10_tty_name, O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd == -1) {
		fprintf(stderr,"Could not open tty '%s'.",x10_tty_name);
	}
	
	
	/* Set the options on the port. */
	
	/* We don't want to block reads. */
	if(fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		fprintf(stderr,"Could not set x10 to non-blocking.");
	}
	
	/* Get the current tty settings. */
	if(tcgetattr(fd, &termios) != 0) {
		fprintf(stderr,"Could not get tty attributes.");
	}
	
	/* Enable receiver. */
	termios.c_cflag |= CLOCAL | CREAD;
	
	/* Set to 8N1. */
	termios.c_cflag &= ~PARENB;
	termios.c_cflag &= ~CSTOPB;
	termios.c_cflag &= ~CSIZE;
	termios.c_cflag |=  CS8;
	
	/* Accept raw data. */
	termios.c_lflag &= ~(ICANON | ECHO | ISIG);
	termios.c_oflag &= ~(OPOST | ONLCR | OCRNL | ONLRET | OFILL);
	termios.c_iflag &= ~(ICRNL | IXON | IXOFF | IMAXBEL);
	
	/* Return after 1 character available */
	termios.c_cc[VMIN]=1;
	
	/* Set the speed of the port. */
	if(cfsetospeed(&termios, B4800) != 0) {
		fprintf(stderr,"Could not set tty output speed.");
	}
	if(cfsetispeed(&termios, B4800) != 0) {
		fprintf(stderr,"Could not set tty input speed.");
	}
	
	/* Save our modified settings back to the tty. */
	if(tcsetattr(fd, TCSANOW, &termios) != 0) {
		fprintf(stderr,"Could not set tty attributes.");
	}
	
	return(fd);
}


/* 
 * Wait for the x10 hardware to provide us with some data.
 *
 * This function should only be called when we know the x10 should have sent
 * us something.  We don't wait long in here, if it isn't screwed up, it
 * should be sending quite quickly.  We return true if we got a byte and
 * false if we timed out waiting for one.
 */
int x10_wait_read(int	fd) {
	fd_set read_fd_set;
	struct timeval tv;
	int retval;
	
	/* Wait for data to be readable. */
	for(;;) {
		
		/* Make the call to select to wait for reading. */
		FD_ZERO(&read_fd_set);
		FD_SET(fd, &read_fd_set);
		tv.tv_sec = (X10_WAIT_READ_USEC_DELAY)/1000000u;
		tv.tv_usec = (X10_WAIT_READ_USEC_DELAY)%1000000u;
		retval=select(fd+1, &read_fd_set, NULL, NULL, &tv);
		
		/* Did select error? */
		if(retval == -1) {
			
			/* If it's an EINTR, go try again. */
			if(errno == EINTR) {
				continue;
			}
			
			/* It was something weird. */
			fprintf(stderr,"Error in read select: %s", strerror(errno));
		}
		
		/* Was data available? */
		if(retval) {	
			
			/* We got some data, return ok. */
			return(1);
		}
		
		/* No data available. */
		else {
			
			/* We didn't get any data, this is a fail. */
			return(0);
		}
	}
}


/* 
 * Wait for the x10 hardware to be writable.
 */
int x10_wait_write(int	fd) {
	fd_set write_fd_set;
	struct timeval tv;
	int retval;
	
	/* Wait for data to be writable. */
	for(;;) {
		
		/* Make the call to select to wait for writing. */
		FD_ZERO(&write_fd_set);
		FD_SET(fd, &write_fd_set);
		tv.tv_sec = (X10_WAIT_WRITE_USEC_DELAY)/1000000u;
		tv.tv_usec = (X10_WAIT_WRITE_USEC_DELAY)%1000000u;
		retval=select(fd+1, NULL, &write_fd_set, NULL, &tv);
		
		/* Did select error? */
		if(retval == -1) {
			
			/* If it's an EINTR, go try again. */
			if(errno == EINTR) {
				continue;
			}
			
			/* It was something weird. */
			fprintf(stderr,"Error in write select: %s", strerror(errno));
		}
		
		/* Can we write data? */
		if(retval) {	
			
			/* We can write some data, return ok. */
			return(1);
		}
		
		/* No data writable. */
		else {
			
			/* We can't write any data, this is a fail. */
			return(0);
		}
	}
}


/* 
 * Read data from the x10 hardware.
 *
 * Basically works like read(), but with a select-provided readable check
 * and timeout.
 * 
 * Returns the number of bytes read.  This might be less than what was given
 * if we ran out of time.
 */
ssize_t x10_read(int fd, void *buf, size_t count) {
	int bytes_read;
	ssize_t retval;
	
	/* 
	 * The x10 sends at maximum 8 data bytes (we don't count the size or
	 * function byte here), so we better not ask for more.
	 */
	if(count > 8) {
		
		/* 
		 * This can actually happen because of the cm11a getting
		 * confused or sending a poll.  We need to handle it
		 * gracefully.
		 */
		return(0);
	}
	
	/* Read the request into the buffer. */
	for(bytes_read=0; bytes_read < count;) {
		
		/* Wait for data to be available. */
		if(!x10_wait_read(fd)) {
			return(bytes_read);
		}
		
		/* Get as much of it as we can.  Loop for the rest. */
		retval=read(fd, (char *) buf + bytes_read, count - bytes_read);
		if(retval == -1) {
			fprintf(stderr,"Failure reading x10 buffer: %s", strerror(errno));
		}
		bytes_read += retval;
	}
	
	/* We're all done. */
	return(bytes_read);
}


/* 
 * Write data to the x10 hardware.
 *
 * Basically works like write(), but with a select-provided writeable check
 * and timeout.
 * 
 * Returns the number of bytes written.  This might be less than what was
 * given if we ran out of time.
 */
ssize_t x10_write(int	fd, void *buf, size_t count) {
	int bytes_written;
	ssize_t retval;
	
	/* Write the buffer to the x10 hardware. */
	for(bytes_written=0; bytes_written < count;) {
		
		/* Wait for data to be writeable. */
		if(!x10_wait_write(fd)) {
			return(bytes_written);
		}
		
		/* Get as much of it as we can.  Loop for the rest. */
		retval=write(fd, (char *) buf + bytes_written, count - bytes_written);
		if(retval == -1) {
			fprintf(stderr,"Failure writing x10 buffer.");
		}
		bytes_written += retval;
	}
	
	/* We're all done. */
	return(bytes_written);
}


/* 
 * Build the time structure to send to the x10 hardware.
 *
 * Note that the download header, 0x9b, is not included here.  That should
 * be handled by the caller if needed.
 */
void x10_build_time(char *buffer, time_t time, int house_code, int flags) {
	struct tm *tm;
	
	/* Break the time given down into day, year, etc.. */
	tm=localtime(&time);
	
	/* Byte zero is the number of seconds. */
	buffer[0]=(char) tm->tm_sec;
	
	/* Byte one is the minutes from 0 to 119. */
	buffer[1]=(char) tm->tm_min;
	if(tm->tm_hour % 2) buffer[1] += (char) 60;
	
	/* Byte two is the hours/2. */
	buffer[2]=(char) tm->tm_hour/2;
	
	/* Byte three and the first bit in four is the year day. */
	buffer[3]=(char) tm->tm_yday & 0xff;
	buffer[4]=(char) (tm->tm_yday >> 8) & 0x1;
	
	/* The top 7 bits of byte 4 are the day mask (SMTWTFS). */
	buffer[4] |= (char) (1 << (tm->tm_wday + 1));
	
	/* The top 4 of byte 5 is the house code. */
	buffer[5]=(char) house_code << 4;
	
	/* One bit is reserved and the lower three are flags. */
	buffer[5] |= (char) flags;

	return;
}


/* 
 * Write a message to the x10 hardware.
 *
 * The data will be sent, a checksum from the x10 hardware will be expected,
 * a response to the checksum will be sent, and the x10 should signal us
 * ready.
 *
 * Sometimes the cm11a will kick into poll mode while we're trying to send
 * it a request then promptly ignore us until we do something about it.  To
 * handle this, if that looks like what is happening, we go deal with the
 * x10 then come back and try again.
 *
 * If it works, we return true, false otherwise.
 */
int x10_write_message(int fd, void *buf, size_t count) {
	unsigned char checksum;
	unsigned char real_checksum;
	unsigned char temp;
	int i;
	int try_count;
	
	/* Try writing the message 5 times, then just fail. */
	for(try_count=1; try_count <= 5; try_count++) {
		printf("pass %d\n",try_count);	
		/* Send the data. */
		if(x10_write(fd, buf, count) != count) {
			continue;
		}
		printf("got here.\n");	
		/* Get the checksum byte from the x10 hardware. */
		if(x10_read(fd, &checksum, 1) != 1) {
			continue;
		}
		
		/* Calculate the checksum on the data.  This is a simple summation. */
		real_checksum=0;
		for(i=0; i < count; i++) {
			real_checksum=(real_checksum + ((char *) buf)[i]) & 0xff;
		}
		
		/* Make sure the checksums match. */
		if(checksum != real_checksum) {
			/* Does this look like it was really a poll? */
			if(checksum == 0x5a) {
				
				/* Go service the x10, it probably needs some. */
				read_x10_poll(fd);
			}
			
			/* Retry sending. */
			continue;
		}
		
		/* Send a go-ahead to the x10 hardware. */
		temp=0;
		if(x10_write(fd, &temp, 1) != 1) {
			continue;
		}
			
		/* Get the ready byte from the x10 hardware. */
		if(x10_read(fd, &temp, 1) != 1) {
			continue;
		}
		
		/* It had better be 0x55, the 'ready' byte. */
		if(temp != 0x55) {
			
			/* Does this look like it was really a poll? */
			if(temp == 0x5a) {
				
				/* Go service the x10, it probably needs some. */
				read_x10_poll(fd);
			}
			
			/* Retry sending. */
			continue;
		}
		
		/* We made it, return true. */
		return(1);
	}
		
	/* We gave up and failed. */
	return(0);
}
