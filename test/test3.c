/* MEMORY2 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "undo_test.h"

int main() {
	UNDO_MEMORY *arena;
	char *a, *b, *c;
	char cmd[256];
	int pid;

	arena = undo_memory_new();
	assert(arena != NULL);

	pid = getpid();
	sprintf(cmd, "cp /proc/%d/maps /tmp/undo.maps.%d", pid, pid);
	system(cmd);

	a = undo_memory_alloc(arena, 16);
	assert(a != NULL);
	b = undo_memory_alloc(arena, 16);
	assert(b != NULL);
	undo_memory_free(arena, a);
	/*  This will make sure block integrity is preserved if we 
	    reallocate a same size block  */
	a = undo_memory_alloc(arena, 16);
	assert(a != NULL);
	c = undo_memory_alloc(arena, 16);
	assert(c != NULL);
	memset(c, 0xff, 16);

	undo_memory_free(arena, a);
	undo_memory_free(arena, c);

	/*  This will make sure block integrity is preserved if we 
	    reallocate a smaller size block  */
	a = undo_memory_alloc(arena, 16 - UNDO_MEMORY_OVERHEAD);
	assert(a != NULL);
	c = undo_memory_alloc(arena, 16);
	assert(c != NULL);
	memset(c, 0xff, 16);

	assert(undo_memory_destroy(arena) == UNDO_NOERROR);

	sprintf(cmd, "cp /proc/%d/maps /tmp/undo.maps_final.%d", pid, pid);
	system(cmd);

	/*  This is a very non-portable way to check and see if all the 
	    memory that was mapped in by undo_memory_alloc is now freed.  
            It'll only work under Linux.

	    Maybe there is a better way?  */
	sprintf(cmd, "cmp /tmp/undo.maps.%d /tmp/undo.maps_final.%d", pid, pid);
	assert(system(cmd) == 0);

	sprintf(cmd, "rm -f /tmp/undo.maps.%d /tmp/undo.maps_final.%d", pid, pid);
	system(cmd);

	return 0;
}
