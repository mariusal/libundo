#ifndef __undo_memory_stream_h
#define __undo_memory_stream_h

#include <stdlib.h>

typedef struct _UNDO_MEMORY_STREAM UNDO_MEMORY_STREAM;
typedef struct _UNDO_MEMORY_STREAM_BLOCK_HEADER 
                   UNDO_MEMORY_STREAM_BLOCK_HEADER;

#define MEMORY_OFFSET(mem, offset) ((void *)((char *)(mem) + (offset)))

struct _UNDO_MEMORY_STREAM {
	void (*destroy)(UNDO_MEMORY_STREAM *stream);
	size_t (*read)(UNDO_MEMORY_STREAM *stream, size_t offset,
				   void *mem, size_t count);
	void *implementation;
};

size_t undo_memory_stream_write(size_t offset, void *mem, size_t size,
								size_t *pos, void *data, size_t length);
size_t undo_memory_stream_length(UNDO_MEMORY_STREAM *stream);

/*  This might not be the same as sizeof(UNDO_MEMORY_STREAM_BLOCK_HEADER)
	if the compiler pads the structure in some way.  This value represents
	the size of the header when it is serialized in a stream.  */
#define STREAM_SERIALIZED_BLOCK_HEADER_SIZE (sizeof(void *) + \
                                    sizeof(size_t) + sizeof(unsigned))
struct _UNDO_MEMORY_STREAM_BLOCK_HEADER {
	void *addr;
	size_t size;
	unsigned flags;
};

size_t undo_memory_stream_read_header(UNDO_MEMORY_STREAM *stream, 
									  size_t offset,
									  UNDO_MEMORY_STREAM_BLOCK_HEADER *header);

#endif
