/*
 *  Copyright (c) 2013 Alexander Wong <admin@alexander-wong.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/user.h>

#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "memchunk.h"

/* Function Prototyping */
int check_RW(char *);
void initiate_handler();
void restore_handler();
void handler(int);

/* Global variables */
static sigjmp_buf sigbuff;	
static struct sigaction newsa, oldsa;

int get_mem_layout(struct memchunk *chunk_list, int size) {
	/*
	 * Scans the entire memory of the calling process, returns within
	 * chunk list the values comprising the chunks. Assumed 32 bit
	 * memory addresses. Iterates by page size, which is portable, sort of.
	 *
	 * libc4, libc5, glibc 2.0 fail because their getpagesize() 
	 * returns a statically derived value, and does not use a system call. 
	 */
	int pagesize = getpagesize();             /* get page size */
	int totalpages = 0xffffffff / pagesize;   /* total num pages for 32bit*/
	int page = 0;                             /* current page */
	int chunkno = 0;                          /* number of chunks */
	char * cur_address = (char*) 0x00000000;  /* cur address, start 0 */
	int cur_rw = 0;                           /* cur address read write */
	int prv_rw = 0;                           /* prev address read write */
	
	for(page = 0; page < totalpages; page++) {
		prv_rw = cur_rw;
		cur_rw = rw_address(cur_address);
		if (cur_rw == -1) {
			/* unreadable, skip page */
			cur_address += pagesize;
			continue;
		}

		if (chunkno < size) {
			if (prv_rw == cur_rw) {
				/* permissions are equivalent */
				chunk_list[chunkno-1].length += PAGE_SIZE;
			} else {
				/* permissions are not equivalent */
				chunk_list[chunkno].start = cur_address;
				chunk_list[chunkno].length = pagesize;
				chunk_list[chunkno].RW = cur_rw;	
				chunkno++;
			}
		} else {
			if (cur_rw != prv_rw)
				/* new chunk */
				chunkno++;	
		}
		cur_address += pagesize;
	}
	return chunkno;
}

int rw_address(char * cur_address) {
	/* 
	 * Returns -1 if no read (do not check for write),
	 * 0 if read only,
	 * 1 if read/write
	 */
	int sigval;
	char data;
	
	initiate_handler();
	sigval = sigsetjmp(sigbuff, 1);
	
	if (sigval == 0) {
		data = *cur_address;
	} else {
		/* cannot read */
		restore_handler();
		return -1;
	}

	sigval = sigsetjmp(sigbuff, 1);

	if (sigval == 0) {
		*cur_address = 'a';
	} else {
		/* cannot write */
		restore_handler();
		return 0;
	}

	*cur_address = data;
	restore_handler();
	return 1;
}

void initiate_handler() {
	/* Modified from Bob Beck's example in class*/
	newsa.sa_handler = handler;
	sigemptyset(&newsa.sa_mask);
	newsa.sa_flags = 0;
	if (sigaction(SIGSEGV, NULL, &oldsa) == -1) {
		err(1, "Cannot save previous sigaction");
	}
	if (sigaction(SIGSEGV, &newsa, NULL) == -1) {
		err(1, "Cannot set next sigaction");
	}
	
}

void restore_handler() {
	/* Modified from Bob Beck's example in class*/
	if (sigaction(SIGSEGV, &oldsa, NULL) == -1) {
		err(1, "Cannot restore previous signal handler");
	}
}

void handler(int signo) {
	/* Modified from Bob Beck's example in class*/
	siglongjmp(sigbuff, 1);
}
