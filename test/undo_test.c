#include "undo_test.h"

typedef struct _TEST_STREAM TEST_STREAM;
struct _TEST_STREAM {
	void *mem;
	unsigned size;
};

void undo_test_destroy(UNDO_MEMORY_STREAM *stream) {
	free(stream->implementation);
	free(stream);
}

size_t undo_test_read(UNDO_MEMORY_STREAM *stream,
					  size_t offset, void *mem, size_t size) {
	size_t pos;
	TEST_STREAM *data;

	data = (TEST_STREAM *)stream->implementation;

	pos = 0;

	return undo_memory_stream_write(offset, mem, size,
									&pos, data->mem, data->size);
}

UNDO_MEMORY_STREAM *undo_test_memory_stream(void *mem, unsigned size) {
	UNDO_MEMORY_STREAM *stream;
	TEST_STREAM *data;

	data = NEW(TEST_STREAM);
	data->mem = mem;
	data->size = size;

	stream = NEW(UNDO_MEMORY_STREAM);
	stream->destroy = undo_test_destroy;
	stream->read = undo_test_read;
	stream->implementation = data;

	return stream;
}
