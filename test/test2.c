/* MEMORY */

#include <string.h>
#include <unistd.h>
#include "undo_test.h"

int main() {
	UNDO_MEMORY *arena;
	char *a, *b;
	size_t large;

	arena = undo_memory_new();
	assert(arena != NULL);
	assert(undo_memory_used(arena) == 0);

	a = undo_memory_alloc(arena, 0);
	assert(a != NULL);
	assert(undo_memory_used(arena) == UNDO_MEMORY_OVERHEAD);
	assert(undo_memory_pages_used(arena) == 1);
	b = undo_memory_alloc(arena, 5);
	assert(b != NULL);
	memset(b, 0, 5);
	assert(undo_memory_size(arena, b) == 8);
	assert(undo_memory_used(arena) == UNDO_MEMORY_OVERHEAD * 2 + 8);
	assert(undo_memory_pages_used(arena) == 1);
	assert(undo_memory_free(arena, a) == UNDO_NOERROR);
	assert(undo_memory_used(arena) == UNDO_MEMORY_OVERHEAD + 8);
	assert(undo_memory_free(arena, b) == UNDO_NOERROR);
	assert(undo_memory_used(arena) == 0);

	large = getpagesize() * 4;
	a = undo_memory_alloc(arena, large);
	assert(a != NULL);
	memset(a, 0, large);
	assert(undo_memory_size(arena, a) == large);
	assert(undo_memory_used(arena) == large);
	assert(undo_memory_free(arena, a) == UNDO_NOERROR);
	assert(undo_memory_used(arena) == 0);

	large = getpagesize() - UNDO_MEMORY_OVERHEAD;
	a = undo_memory_alloc(arena, large);
	assert(a != NULL);
	memset(a, 42, large);
	assert(undo_memory_free(arena, a) == UNDO_NOERROR);

	assert(undo_memory_destroy(arena) == UNDO_NOERROR);

	return 0;
}
