/* UNDO_API2 */

#include "undo_test.h"

int main() {
	UNDO *a, *b, *c;

	a = undo_new();
	assert(a != NULL);
	b = undo_new();
	assert(b != NULL);
	c = undo_new();
	assert(c != NULL);
	assert(undo_destroy(b) == UNDO_NOERROR);
	b = undo_new();
	assert(undo_destroy(b) == UNDO_NOERROR);
	assert(undo_destroy(c) == UNDO_NOERROR);
	assert(undo_destroy(a) == UNDO_NOERROR);

	return 0;
}
