/* UNDO_API_VERSION */

#include "undo_test.h"

int main() {
	unsigned major, minor, revision;
	undo_get_version(&major, &minor, &revision);
	assert(major == UNDO_MAJOR_VERSION);
	assert(minor == UNDO_MINOR_VERSION);
	assert(revision == UNDO_REVISION);

	return 0;
}
