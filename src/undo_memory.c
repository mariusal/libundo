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

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "undoP.h"

#define BLOCK_FLAG_LARGE	0x01

#define MEMORY_RAW_SIZE(mem)	(*(size_t *)(mem))
#define MEMORY_USED(mem)		(MEMORY_RAW_SIZE(mem) & 1)
#define MEMORY_SIZE(mem)		(MEMORY_RAW_SIZE(mem) & ~1)

#define MEMORY_SET_SIZE_USED(mem, size, used) \
			(MEMORY_RAW_SIZE(mem) = ((size) & ~1) | ((used) ? 1 : 0))

#define MEMORY_NEXT(mem) (MEMORY_OFFSET((mem), MEMORY_SIZE(mem)))
#define MEMORY_BODY(mem) (MEMORY_OFFSET((mem), UNDO_MEMORY_OVERHEAD))

#define FOREACH_SIZED_BLOCK(mem, size, block_ix, block) \
			for((block_ix) = 0, (block) = &(mem)->size##_alloc_list[0]; \
				(block_ix) < (mem)->size##_alloc_list_count; \
				(block_ix)++, (block) = &(mem)->size##_alloc_list[(block_ix)])
#define FOREACH_SMALL_BLOCK(mem, block_ix, block) \
			FOREACH_SIZED_BLOCK(mem, small, block_ix, block)
#define FOREACH_LARGE_BLOCK(mem, block_ix, block) \
			FOREACH_SIZED_BLOCK(mem, large, block_ix, block)

#define MEMORY_BACKUP_OVERHEAD(mem) \
			((mem) = MEMORY_OFFSET((mem), -UNDO_MEMORY_OVERHEAD))

#define BLOCK_END(block) MEMORY_OFFSET((block).mem, (block).size)
#define IN_BLOCK(block, m) ((m) >= (block).mem && (m) < BLOCK_END(block))
#define BLOCK_PAGES(mem, block) ((block).size / mem->pagesize)
#define FOREACH_IN_BLOCK(block, mem) \
			for((mem) = (block).mem; \
				(mem) != BLOCK_END(block); \
				(mem) = MEMORY_NEXT(mem))

#define MAP_NEW_ANON_AT_FLAGS(pos, size, flags) \
            (mmap((pos), (size), \
			      PROT_READ | PROT_WRITE | PROT_EXEC, \
				  MAP_PRIVATE | MAP_ANONYMOUS | (flags), 0, 0))
#define MAP_NEW_ANON(size) MAP_NEW_ANON_AT_FLAGS(0, (size), 0)
#define MAP_NEW_ANON_AT(pos, size) \
            MAP_NEW_ANON_AT_FLAGS((pos), (size), MAP_FIXED)
#define PAGE_REMAINDER(mem, size) (mem->pagesize - (size))

#define IF_IN_BLOCK_VAR \
			unsigned _if_in_block_ix; \
			UNDO_BLOCK *_if_in_block
#define IF_IN_SMALL_BLOCK(memory, ptr) \
			FOREACH_SMALL_BLOCK((memory), _if_in_block_ix, _if_in_block) \
				if(IN_BLOCK(*_if_in_block, (ptr)))
#define IF_IN_LARGE_BLOCK(memory, ptr) \
			FOREACH_LARGE_BLOCK((memory), _if_in_block_ix, _if_in_block) \
				if((ptr) == _if_in_block->mem)

static void *undo_memory_alloc_small(UNDO_MEMORY *memory, size_t size);
static void *undo_memory_alloc_large(UNDO_MEMORY *memory, size_t size);
static void *undo_memory_alloc_small_block(UNDO_BLOCK *block, size_t size);
static void undo_memory_block_coalesce_free(UNDO_BLOCK *block);
static void *undo_memory_alloc_new_small_block(UNDO_MEMORY *mem, size_t size);
static void *undo_memory_new_block(UNDO_BLOCK **block_list, unsigned *length,
								   size_t size);
static int undo_memory_new_block_record(UNDO_BLOCK **block_list, unsigned *len,
										void *mem, size_t size);
static int undo_memory_delete_block(UNDO_BLOCK **block_list, unsigned *length,
									unsigned index);

static void undo_stream_destroy(UNDO_MEMORY_STREAM *stream);
static size_t undo_stream_read(UNDO_MEMORY_STREAM *stream, size_t offset,
							   void *mem, size_t size);
static size_t undo_stream_block_write(size_t offset, void *mem, size_t size,
									  size_t *pos, 
									  UNDO_BLOCK *block, unsigned flags);
static void undo_memory_clear(UNDO_MEMORY *memory);
static int undo_memory_add_blocks_from_stream(UNDO_MEMORY *memory,
											  UNDO_MEMORY_STREAM *stream);
static void undo_memory_block_header_record(UNDO_MEMORY *memory,
									UNDO_MEMORY_STREAM_BLOCK_HEADER *header);

UNDO_MEMORY *undo_memory_new(void) {
	UNDO_MEMORY *mem;

	mem = NEW(UNDO_MEMORY);
	if (mem) {
		mem->pagesize = getpagesize();
	}

	return mem;
}

int undo_memory_destroy(UNDO_MEMORY *memory) {
	if(memory == NULL)
		return UNDO_BADPARAM;

	undo_memory_clear(memory);  
	free(memory);

	UNDO_SUCCESS;
}

size_t undo_memory_used(UNDO_MEMORY *memory) {
	size_t used;
	unsigned block_ix;
	UNDO_BLOCK *block;

	used = 0;
	FOREACH_SMALL_BLOCK(memory, block_ix, block) {
		void *mem;

		FOREACH_IN_BLOCK(*block, mem) {
			if(MEMORY_USED(mem)) {
				used += MEMORY_SIZE(mem);
			}
		}
	}

	FOREACH_LARGE_BLOCK(memory, block_ix, block) {
		used += block->size;
	}

	return used;
}

void *undo_memory_alloc(UNDO_MEMORY *memory, size_t size) {
	while(size & (sizeof(size_t) - 1))
		size++;

	if(size < memory->pagesize - UNDO_MEMORY_OVERHEAD * 2) {
		return undo_memory_alloc_small(memory, size);
	} else {
		return undo_memory_alloc_large(memory, size);
	}
}

int undo_memory_free(UNDO_MEMORY *memory, void *alloc) {
	IF_IN_BLOCK_VAR;

	IF_IN_SMALL_BLOCK(memory, alloc) {
		MEMORY_BACKUP_OVERHEAD(alloc);
		MEMORY_SET_SIZE_USED(alloc, MEMORY_SIZE(alloc), 0);
		UNDO_SUCCESS;
	}

	IF_IN_LARGE_BLOCK(memory, alloc) {
		return undo_memory_delete_block(&memory->large_alloc_list,
										&memory->large_alloc_list_count,
										_if_in_block_ix);
	}

	return UNDO_BADPARAM;
}

size_t undo_memory_size(UNDO_MEMORY *memory, void *alloc) {
	IF_IN_BLOCK_VAR;

	IF_IN_SMALL_BLOCK(memory, alloc) {
		MEMORY_BACKUP_OVERHEAD(alloc);
		return MEMORY_SIZE(alloc) - UNDO_MEMORY_OVERHEAD;
	}

	IF_IN_LARGE_BLOCK(memory, alloc) {
		return _if_in_block->size;
	}

	return 0;
}

unsigned undo_memory_pages_used(UNDO_MEMORY *memory) {
	unsigned block_ix;
	UNDO_BLOCK *block;
	unsigned pages;

	pages = 0;

	FOREACH_SMALL_BLOCK(memory, block_ix, block) {
		pages += BLOCK_PAGES(memory, *block);
	}

	FOREACH_LARGE_BLOCK(memory, block_ix, block) {
		pages += BLOCK_PAGES(memory, *block);
	}

	return pages;
}

UNDO_MEMORY_STREAM *undo_memory_stream(UNDO_MEMORY *memory) {
	UNDO_MEMORY_STREAM *stream;

	stream = NEW(UNDO_MEMORY_STREAM);
	if(stream == NULL)
		return NULL;

	stream->implementation = memory;
	stream->destroy = (void *)undo_stream_destroy;
	stream->read = (void *)undo_stream_read;

	return (UNDO_MEMORY_STREAM *)stream;
}

int undo_memory_set(UNDO_MEMORY *memory, UNDO_MEMORY_STREAM *stream) {
	undo_memory_clear(memory);
	return undo_memory_add_blocks_from_stream(memory, stream);
}

static void *undo_memory_alloc_small(UNDO_MEMORY *memory, size_t size) {
	unsigned block_ix;
	UNDO_BLOCK *block;

	FOREACH_SMALL_BLOCK(memory, block_ix, block) {
		void *mem;

		mem = undo_memory_alloc_small_block(block, size);
		if(mem != NULL) {
			return mem;
		}
	}

	return undo_memory_alloc_new_small_block(memory, size);
}

static void *undo_memory_alloc_large(UNDO_MEMORY *memory, size_t size) {
	size_t page_fraction;

	page_fraction = size & (memory->pagesize - 1);
	if(page_fraction) {
		size += PAGE_REMAINDER(memory, page_fraction);
	}

	return undo_memory_new_block(&memory->large_alloc_list,
								 &memory->large_alloc_list_count,
								 size);
}

static void *undo_memory_alloc_small_block(UNDO_BLOCK *block, size_t size) {
	void *mem;
	size_t new_size;

	undo_memory_block_coalesce_free(block);

	new_size = size + UNDO_MEMORY_OVERHEAD;
	FOREACH_IN_BLOCK(*block, mem) {
		if(!MEMORY_USED(mem) && MEMORY_SIZE(mem) >= new_size) {
			if(MEMORY_SIZE(mem) == new_size) {
				MEMORY_SET_SIZE_USED(mem, new_size, 1);
			} else {
				size_t old_size;
				void *next;

				old_size = MEMORY_SIZE(mem);
				MEMORY_SET_SIZE_USED(mem, new_size, 1);
				next = MEMORY_NEXT(mem);
				MEMORY_SET_SIZE_USED(next, old_size - new_size, 0);
			}
			return MEMORY_BODY(mem);
		}
	}

	return NULL;
}

static void undo_memory_block_coalesce_free(UNDO_BLOCK *block) {
	void *last, *mem;

	last = NULL;
	FOREACH_IN_BLOCK(*block, mem) {
		if(last == NULL) {
			last = mem;
			continue;
		}

		if(!MEMORY_USED(last) && !MEMORY_USED(mem)) {
			MEMORY_SET_SIZE_USED(last, 
								 MEMORY_SIZE(last) + MEMORY_SIZE(mem),
								 0);
			continue;
		}

		last = mem;
	}
}

static void *undo_memory_alloc_new_small_block(UNDO_MEMORY *mem, size_t size) {
	void *page, *next;

	page = undo_memory_new_block(&mem->small_alloc_list, 
								 &mem->small_alloc_list_count,
								 mem->pagesize);
	if(page == NULL) {
		return NULL;
	}

	MEMORY_SET_SIZE_USED(page, size + UNDO_MEMORY_OVERHEAD, 1);
	next = MEMORY_NEXT(page);
	MEMORY_SET_SIZE_USED(next,
		PAGE_REMAINDER(mem, size + UNDO_MEMORY_OVERHEAD), 0);

	return MEMORY_BODY(page);
}

static void *undo_memory_new_block(UNDO_BLOCK **block_list, unsigned *len,
								   size_t size) {
	void *mem;

	mem = MAP_NEW_ANON(size);
	if(mem == MAP_FAILED)
		return NULL;

	if(undo_memory_new_block_record(block_list, len, mem, size)) {
		munmap(mem, size);
		return NULL;
	}

	return mem;
}

static int undo_memory_new_block_record(UNDO_BLOCK **block_list, unsigned *len,
										void *mem, size_t size) {
	UNDO_BLOCK *new_block_list;

	
	new_block_list = (UNDO_BLOCK *)realloc(*block_list, 
										   (*len + 1) * sizeof(UNDO_BLOCK));
	if(new_block_list == NULL) {
		return UNDO_NOMEM;
	}
	*block_list = new_block_list;
	(*block_list)[*len].mem = mem;
	(*block_list)[*len].size = size;
	(*len)++;

	UNDO_SUCCESS;
}

static int undo_memory_delete_block(UNDO_BLOCK **block_list, unsigned *length,
									unsigned index) {
	UNDO_BLOCK *block;
	UNDO_BLOCK *new_block_list;

	block = &(*block_list)[index];
	munmap(block->mem, block->size);

	if(*length == 1) {
		free(*block_list);
		new_block_list = NULL;
	} else {
		memcpy(&(*block_list)[index], &(*block_list)[index + 1],
			   sizeof(UNDO_BLOCK) * (*length - 1 - index));
		new_block_list = realloc(*block_list, 
								 sizeof(UNDO_BLOCK) * (*length - 1));
		if(new_block_list == NULL) {
			(*length)--;
			UNDO_SUCCESS;
		}
	}
	*block_list = new_block_list;
	(*length)--;  

	UNDO_SUCCESS;
}


static void undo_stream_destroy(UNDO_MEMORY_STREAM *stream) {
	free(stream);
}

static size_t undo_stream_read(UNDO_MEMORY_STREAM *stream, size_t offset,
							   void *mem, size_t size) {
	unsigned block_ix;
	UNDO_BLOCK *block;
	UNDO_MEMORY *memory;
	size_t ret;
	size_t pos;
	unsigned flags;

	memory = (UNDO_MEMORY *)stream->implementation;

	ret = 0;
	pos = 0;
	flags = 0;
	FOREACH_SMALL_BLOCK(memory, block_ix, block) {
		ret += undo_stream_block_write(offset, mem, size, 
									   &pos, block, flags);
	}

	flags = BLOCK_FLAG_LARGE;
	FOREACH_LARGE_BLOCK(memory, block_ix, block) {
		ret += undo_stream_block_write(offset, mem, size, 
									   &pos, block, flags); 
	}

	return ret;
}

static size_t undo_stream_block_write(size_t offset, void *mem, size_t size,
									  size_t *pos, 
									  UNDO_BLOCK *block, unsigned flags) {
	size_t ret;

	ret = 0;
	ret += undo_memory_stream_write(offset, mem, size,
									pos, &block->mem, sizeof(void *));
	ret += undo_memory_stream_write(offset, mem, size,
									pos, &block->size, sizeof(size_t));
	ret += undo_memory_stream_write(offset, mem, size,
									pos, &flags, sizeof(unsigned));
	ret += undo_memory_stream_write(offset, mem, size, 
									pos, block->mem, block->size);

	return ret;
}

static void undo_memory_clear(UNDO_MEMORY *memory) {
	unsigned block_ix;
	UNDO_BLOCK *block;

	FOREACH_SMALL_BLOCK(memory, block_ix, block) {
		munmap(block->mem, block->size);
	}
	FOREACH_LARGE_BLOCK(memory, block_ix, block) {
		munmap(block->mem, block->size);
	}

	memset(memory, 0, sizeof(UNDO_MEMORY));
}

static int undo_memory_add_blocks_from_stream(UNDO_MEMORY *memory,
											  UNDO_MEMORY_STREAM *stream) {
	size_t pos, num_read;
	UNDO_MEMORY_STREAM_BLOCK_HEADER header;

	pos = 0;
	do {
		num_read = undo_memory_stream_read_header(stream, pos, &header);
		if(num_read != STREAM_SERIALIZED_BLOCK_HEADER_SIZE) {
			UNDO_SUCCESS;
		}
		pos += num_read;

		if(MAP_NEW_ANON_AT(header.addr, header.size) == MAP_FAILED) {
			return UNDO_NOMEM;
		}
		undo_memory_block_header_record(memory, &header);

		num_read = stream->read(stream, pos, header.addr, header.size);
		pos += num_read;
	} while(1);
}

static void undo_memory_block_header_record(UNDO_MEMORY *memory,
									UNDO_MEMORY_STREAM_BLOCK_HEADER *header) {
	UNDO_BLOCK **undo_block_list;
	unsigned *undo_block_list_count;

	if(header->flags & BLOCK_FLAG_LARGE) {
		undo_block_list = &memory->large_alloc_list;
		undo_block_list_count = &memory->large_alloc_list_count;
	} else {
		undo_block_list = &memory->small_alloc_list;
		undo_block_list_count = &memory->small_alloc_list_count;
	}

	undo_memory_new_block_record(undo_block_list, undo_block_list_count,
								 header->addr, header->size);
}
