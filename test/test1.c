/* STREAM */

#include <string.h>
#include "undo_test.h"

int main() {
	char big[16384];
	char little[16];
	size_t ret, pos;	
	UNDO_MEMORY_STREAM *stream;
	
	memset(big, 0x13, 16384);
	memset(little, 0x14, 16);

	pos = 0;
	ret = undo_memory_stream_write(64, little, 8, 
				       &pos, big, 16384);
	assert(pos == 16384);
	assert(ret == 8);

	undo_memory_stream_write(65536, little, 8, 
				 &pos, big, 16384);

	pos = 80;
	ret = undo_memory_stream_write(64, little, 16, 
				 &pos, big, 16384);
	assert(pos == 16384 + 80);
	assert(ret == 0);

	assert(little[7] == 0x13);
	assert(little[8] == 0x14);

	stream = undo_test_memory_stream(big, 16384);
	assert(undo_memory_stream_length(stream) == 16384);
	stream->destroy(stream);

	return 0;
}
