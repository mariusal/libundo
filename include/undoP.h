/*
    libundo, an easy to use undo/redo management library
    Copyright 1999 Matt Kimball

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __undo_private_h
#define __undo_private_h

#include "undo.h"

#define NEW(type) ((type *)calloc(1, sizeof(type)))

/* undo_error.c */
#define UNDO_SUCCESS               return UNDO_NOERROR;
#define UNDO_ERROR(undo, err)      return (undo_set_error(undo, err), (err));
#define UNDO_ERROR_NULL(undo, err) return (undo_set_error(undo, err), NULL);

void undo_set_error(UNDO *undo, int err);

/* undo_memory_stream.c */
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

/* undo_history.c */
typedef struct _UNDO_HISTORY UNDO_HISTORY;
typedef struct _UNDO_HISTORY_ITEM UNDO_HISTORY_ITEM;

struct _UNDO_HISTORY {
	UNDO_HISTORY_ITEM *item;
	unsigned length;
	unsigned ix;
	size_t memory_limit;
        int logical;
};

struct _UNDO_HISTORY_ITEM {
	void *mem;
	size_t size;
};

UNDO_HISTORY *undo_history_new(void);
int undo_history_destroy(UNDO_HISTORY *history);

int undo_history_record(UNDO_HISTORY *history, UNDO_MEMORY_STREAM *stream);

UNDO_MEMORY_STREAM *undo_history_undo(UNDO_HISTORY *history);
UNDO_MEMORY_STREAM *undo_history_redo(UNDO_HISTORY *history);

unsigned undo_history_undo_count(UNDO_HISTORY *history);
unsigned undo_history_redo_count(UNDO_HISTORY *history);

void undo_history_set_memory_limit(UNDO_HISTORY *history, size_t limit);
void undo_history_set_logical(UNDO_HISTORY *history, int onoff);
size_t undo_history_memory_usage(UNDO_HISTORY *history);

/* undo_memory.c */
typedef struct _UNDO_BLOCK UNDO_BLOCK;
typedef struct _UNDO_MEMORY UNDO_MEMORY;

struct _UNDO_BLOCK {
	void *mem;
	size_t size;
};

struct _UNDO_MEMORY {
	UNDO_BLOCK *small_alloc_list;
	unsigned small_alloc_list_count;

	UNDO_BLOCK *large_alloc_list;
	unsigned large_alloc_list_count;
};

UNDO_MEMORY *undo_memory_new(void);
int undo_memory_destroy(UNDO_MEMORY *memory);

size_t undo_memory_used(UNDO_MEMORY *memory);
unsigned undo_memory_pages_used(UNDO_MEMORY *memory);

void *undo_memory_alloc(UNDO_MEMORY *memory, size_t size);
int undo_memory_free(UNDO_MEMORY *memory, void *alloc);
size_t undo_memory_size(UNDO_MEMORY *memory, void *alloc);

UNDO_MEMORY_STREAM *undo_memory_stream(UNDO_MEMORY *memory);
int undo_memory_set(UNDO_MEMORY *memory, UNDO_MEMORY_STREAM *stream);

#define UNDO_MEMORY_OVERHEAD (sizeof(size_t))

struct _UNDO {
	UNDO_MEMORY *memory;
	UNDO_HISTORY *history;
        
        int last_error;
};

#endif
