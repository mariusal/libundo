/* HISTORY2 */

#include <string.h>
#include "undo_test.h"

int main() {
	UNDO_HISTORY *history;
	UNDO_MEMORY_STREAM *stream;
	char mem[4096];
	int i;

	memset(mem, 0x82, 4096);
	
	history = undo_history_new();
	assert(history != NULL);

	undo_history_set_memory_limit(history, 12000);

	for(i = 0; i < 16384; i++) {
		stream = undo_test_memory_stream(mem, 4096);
		assert(undo_history_record(history, stream) == UNDO_NOERROR);
		stream->destroy(stream);
		assert(undo_history_memory_usage(history) <= 12000);
	}

	assert(undo_history_destroy(history) == UNDO_NOERROR);

	return 0;
}
