/* MEMORY3 */

#include <string.h>
#include <unistd.h>
#include "undo_test.h"

int main() {
	UNDO_MEMORY *arena;
	UNDO_MEMORY_STREAM *stream;
	char *a, *b;
	int count, add_count;
	char buff[4096];
	int stream_count;
	char *mem;

	arena = undo_memory_new();
	assert(arena != NULL);

	a = undo_memory_alloc(arena, 16);
	assert(a != NULL);
	b = undo_memory_alloc(arena, getpagesize() * 3 / 2);
	assert(b != NULL);
	assert(undo_memory_pages_used(arena) == 3);

	memset(a, 0x01, 16);
	memset(b, 0x02, getpagesize() * 3 / 2);
	
	stream = undo_memory_stream(arena);
	count = 0;
	do {
		add_count = stream->read(stream, count, buff, 4096);
		count += add_count;
	} while(add_count);
	assert(count > 0);
	stream_count = count;
	stream->destroy(stream);

	stream = undo_memory_stream(arena);
	mem = (char *)malloc(stream_count);
	count = stream->read(stream, 0, mem, stream_count);
	assert(count == stream_count);
	stream->destroy(stream);

	mem[STREAM_SERIALIZED_BLOCK_HEADER_SIZE * 2 + getpagesize() + 1] = 0x04;

	stream = undo_test_memory_stream(mem, stream_count);
	assert(undo_memory_set(arena, stream) == UNDO_NOERROR);
	stream->destroy(stream);
	assert(a[0] == 0x01);
	assert(a[1] == 0x01);
	assert(b[0] == 0x02);
	assert(b[1] == 0x04);   
	
	assert(undo_memory_destroy(arena) == UNDO_NOERROR);

	return 0;
}
