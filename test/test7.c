/* UNDO_API */

#include "undo_test.h"

int main() {
	UNDO *undo;
	assert((undo = undo_new()) != NULL);
	assert(undo_destroy(undo) == UNDO_NOERROR);
	
	return 0;
}
