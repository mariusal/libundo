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

#ifndef __undo_memory_h
#define __undo_memory_h

#include <stdlib.h>

#include "undo_memory_stream.h"

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

#endif
