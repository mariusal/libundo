/* UNDO_API4 */

#include <string.h>
#include "undo_test.h"

int main() {
	UNDO *undo;
	unsigned char *a;

	assert((undo = undo_new()) != NULL);
	assert(undo_set_memory_limit(undo, 1024 * 1024) == UNDO_NOERROR);

	a = undo_malloc(undo, 42);
	assert(a != NULL);
	memset(a, 0x42, 42);
	assert(undo_get_undo_count(undo) == 0);
	assert(undo_get_redo_count(undo) == 0);
	assert(undo_snapshot(undo) == UNDO_NOERROR);
	assert(undo_get_undo_count(undo) == 0);
	assert(undo_get_redo_count(undo) == 0);
	assert(a[1] == 0x42);
	memset(a, 0x24, 42);
	assert(undo_snapshot(undo) == UNDO_NOERROR);
	assert(undo_get_undo_count(undo) == 1);
	assert(undo_get_redo_count(undo) == 0);
	assert(a[2] == 0x24);
	memset(a, 0x14, 42);
	assert(undo_snapshot(undo) == UNDO_NOERROR);
	assert(undo_get_undo_count(undo) == 2);
	assert(undo_get_redo_count(undo) == 0);

	assert(undo_undo(undo) == UNDO_NOERROR);
	assert(undo_get_undo_count(undo) == 1);
	assert(undo_get_redo_count(undo) == 1);
	assert(a[3] == 0x24);
	assert(undo_undo(undo) == UNDO_NOERROR);
	assert(undo_get_undo_count(undo) == 0);
	assert(undo_get_redo_count(undo) == 2);
	assert(a[4] == 0x42);
	assert(undo_undo(undo) != UNDO_NOERROR);
	assert(undo_redo(undo) == UNDO_NOERROR);
	assert(a[5] == 0x24);
	assert(undo_redo(undo) == UNDO_NOERROR);
	assert(a[6] == 0x14);
	assert(undo_redo(undo) != UNDO_NOERROR);

	assert(undo_undo(undo) == UNDO_NOERROR);
	assert(a[3] == 0x24);
	assert(a[2] == 0x24);
	memset(a, 0x04, 42);
	assert(undo_snapshot(undo) == UNDO_NOERROR);
	assert(undo_undo(undo) == UNDO_NOERROR);
	assert(a[3] == 0x14);
	assert(undo_undo(undo) == UNDO_NOERROR);
	assert(a[3] == 0x24);
	assert(undo_undo(undo) == UNDO_NOERROR);
	assert(a[3] == 0x42);
	assert(undo_undo(undo) != UNDO_NOERROR);

	assert(undo_destroy(undo) == UNDO_NOERROR);

	return 0;
}
