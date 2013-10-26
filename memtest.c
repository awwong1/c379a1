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

/* Program to test memscan.c */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include "memchunk.c"

/* Function Prototyping */
void test_addresses(struct memchunk *);

/* Global Variables */
int varglobal;

int main(int argc, char * argv[]) {
	int num_chunks = 1; /* test will accomodate for any positive int here */
	int chunks_scanned;
	int right_num_chunks;
	int i;
	int pagesize = getpagesize();
	
	struct memchunk * chunk_list = 
	    malloc(sizeof(struct memchunk) * num_chunks);
	if (chunk_list == NULL) {
		err(1, "Could not allocate memory.");
	}
	chunks_scanned = get_mem_layout(chunk_list, num_chunks);
	if (chunks_scanned != num_chunks) {
		num_chunks = chunks_scanned;
		chunk_list = realloc(chunk_list, num_chunks * 
		    sizeof(struct memchunk));
		if (chunk_list == NULL) {
			err(1, "Could not allocate memory.");
		}
		get_mem_layout(chunk_list, num_chunks);
	}
	
	printf("\nCount of chunks scanned: %d\n\n", chunks_scanned);
	
	/** print chunk information **/
	for (i = 0; i < chunks_scanned; i++) {
		char * rw;
		if (chunk_list[i].RW == 0) {
			rw = "Read Only";	
		} else {
			rw = "Read Writable";
		}
		printf("Chunk number %d:\n", i + 1);
		printf("\tStarting address: %p\n", chunk_list[i].start);
		printf("\tChunk length (bytes): %ld\n", chunk_list[i].length);
		printf("\tChunk length (pages): %ld\n", 
		    chunk_list[i].length/pagesize);
		printf("\tRead/Write: %s\n\n", rw);
	}

	test_addresses(chunk_list);
	free(chunk_list);
	return 0;
}

void test_addresses(struct memchunk * chunk_list) {

  int *sam;
  int george;
  void *p1, *p2;
  p1 = malloc(1*sizeof(int));
  p2= malloc;
  sam = ((int*)p1)+4;

	int varlocal = 0;
	int pagesize = getpagesize();
	int totalpages = 0xffffffff / pagesize; /* total num pages, 32bit*/
	
	printf("Chunk guesstimations...\n");
	printf("\tAddress of main (text): %p\n", &main);
	printf("\tAddress of aglobal (data): %p\n", &varglobal);
	printf("\tAddress of alocal (stack): %p\n", &varlocal);
	printf("\tAddress of chunk_list (heap address): %p\n", chunk_list);
	printf("\tPagesize: %d\n", pagesize);
	printf("\tTotalpages: %d\n", totalpages);

	printf("\tGeorge: %p\n", &george);
	//printf("\tadSam: &d\n", &&sam);
	printf("\tSam: %p\n", &sam);
	printf("\tp1: %p\n", &p1);
	printf("\tp2: %p\n", &p2);
	
	
	
}

