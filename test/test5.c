/* HISTORY */

#include <string.h>
#include "undo_test.h"

int main() {
	UNDO_HISTORY *history;
	UNDO_MEMORY_STREAM *stream;
	char orig[4096], copy[4096];
	int at;

	for(at = 0; at < 4096; at++) {
		orig[at] = at;
	}

	history = undo_history_new();
	assert(history != NULL);
	undo_history_set_memory_limit(history, 1024 * 1024);

	stream = undo_test_memory_stream(orig, 4096);
	assert(undo_history_record(history, stream) == UNDO_NOERROR);
	stream->destroy(stream);

	for(at = 0; at < 4096; at++) {
		orig[at] = 4096 - at;
	}

	stream = undo_test_memory_stream(orig, 4096);
	assert(undo_history_record(history, stream) == UNDO_NOERROR);
	stream->destroy(stream);

	stream = undo_history_undo(history);
	assert(stream != NULL);
	stream->read(stream, 0, copy, 4096);
	stream->destroy(stream);
	assert(memcmp(orig, copy, 4096));

	stream = undo_history_undo(history);
	assert(stream == NULL);

	stream = undo_history_redo(history);
	assert(stream != NULL);
	stream->read(stream, 0, copy, 4096);
	stream->destroy(stream);
	assert(!memcmp(orig, copy, 4096));

	assert(undo_history_destroy(history) == UNDO_NOERROR);

	return 0;
}
