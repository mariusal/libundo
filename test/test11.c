/* UNDO_API_ERROR */

#include "undo_test.h"

int main() {
	assert(undo_malloc(NULL, 42) == NULL);
	assert(undo_get_errcode(NULL) == UNDO_NOSESSION);
	assert(undo_strerror(-1) == NULL);
	assert(undo_strerror(4242) == NULL);
	assert(undo_strerror(UNDO_NOSESSION) != NULL);

	return 0;
}
