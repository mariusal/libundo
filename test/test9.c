/* UNDO_API3 */

#include <string.h>
#include "undo_test.h"

int main() {
	UNDO *undo;
	unsigned char *a, *b;

	assert((undo = undo_new()) != NULL);

	a = undo_malloc(undo, 42);
	assert(a != NULL);
	memset(a, 0xff, 42);
	
	b = undo_malloc(undo, 16384);
	assert(b != NULL);
	memset(b, 0xee, 16384);

	a = undo_realloc(undo, a, 16384); 
	assert(a != NULL);

	assert(a[13] == 0xff);
	assert(b[12] == 0xee);

	memset(a, 0xff, 16384);

	undo_free(undo, a);
	undo_free(undo, b);

	assert(undo_destroy(undo) == UNDO_NOERROR);

	return 0;	
}
