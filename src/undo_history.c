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
#include <string.h>

#include "undo.h"
#include "undo_history.h"
#include "undo_memory_stream.h"
#include "undo_error.h"

static void undo_history_stream_destroy(UNDO_MEMORY_STREAM *stream);
static size_t undo_history_stream_read(UNDO_MEMORY_STREAM *stream, 
									   size_t offset,
									   void *mem, size_t size);
static int undo_history_add_item(UNDO_HISTORY *history, 
								 void *mem, size_t size);
static UNDO_MEMORY_STREAM *undo_history_stream(UNDO_HISTORY *history);
static void undo_history_shrink(UNDO_HISTORY *history, size_t size);
static void undo_history_strip(UNDO_HISTORY *history, int count);
static void undo_history_strip_trailing(UNDO_HISTORY *history);

UNDO_HISTORY *undo_history_new(void) {
	UNDO_HISTORY *history;

	history = NEW(UNDO_HISTORY);
	if(history == NULL)
		UNDO_ERROR_NULL(UNDO_NOMEM);

	return history;
}

int undo_history_destroy(UNDO_HISTORY *history) {
	if(history == NULL)
		UNDO_ERROR(UNDO_BADPARAM);

	free(history);

	UNDO_SUCCESS;
}

int undo_history_record(UNDO_HISTORY *history, UNDO_MEMORY_STREAM *stream) {
	void *mem;
	size_t size;
	int ret;

	if(history->memory_limit == 0)
		UNDO_ERROR(UNDO_NOLIMIT);

	size = undo_memory_stream_length(stream);
	if(size > history->memory_limit) {
		undo_history_shrink(history, 0);
		UNDO_SUCCESS;
	}
	
	if(history->logical)
		undo_history_strip_trailing(history);
    
	undo_history_shrink(history, history->memory_limit - size);

	mem = (void *)malloc(size);
	if(mem == NULL)
		UNDO_ERROR(UNDO_NOMEM);
	stream->read(stream, 0, mem, size);

	ret = undo_history_add_item(history, mem, size);
	if(ret) {
		free(mem);
	}

	return ret;
}

UNDO_MEMORY_STREAM *undo_history_undo(UNDO_HISTORY *history) {
	UNDO_MEMORY_STREAM *stream;

	if(history->ix == 0)
		UNDO_ERROR_NULL(UNDO_NODO);

	stream = undo_history_stream(history);

	if(stream)
		history->ix--;

	return stream; 
}

UNDO_MEMORY_STREAM *undo_history_redo(UNDO_HISTORY *history) {
	UNDO_MEMORY_STREAM *stream;

	if(history->ix >= history->length - 1)
		UNDO_ERROR_NULL(UNDO_NODO);

	stream = undo_history_stream(history);

	if(stream)
		history->ix++;

	return stream; 
}

unsigned undo_history_undo_count(UNDO_HISTORY *history) {
	if(history->length)
		return history->ix;

	return 0;
}

unsigned undo_history_redo_count(UNDO_HISTORY *history) {
	if(history->length) 
		return history->length - 1 - history->ix;

	return 0;
}

void undo_history_set_memory_limit(UNDO_HISTORY *history, size_t limit) {
	history->memory_limit = limit;
}

void undo_history_set_logical(UNDO_HISTORY *history, int onoff) {
	history->logical = onoff;
}

size_t undo_history_memory_usage(UNDO_HISTORY *history) {
	int ix;
	size_t total;

	total = 0;
	for(ix = 0; ix < history->length; ix++) {
		total += history->item[ix].size;
	}

	return total;
}

static void undo_history_stream_destroy(UNDO_MEMORY_STREAM *stream) {
	free(stream);
}

static size_t undo_history_stream_read(UNDO_MEMORY_STREAM *stream, 
									   size_t offset,
									   void *mem, size_t size) {
	UNDO_HISTORY *history;
	size_t pos;
	void *history_mem;
	size_t history_size;

	history = (UNDO_HISTORY *)stream->implementation;

	pos = 0;
	history_mem = history->item[history->ix].mem;
	history_size = history->item[history->ix].size;
	return undo_memory_stream_write(offset, mem, size,
									&pos, history_mem, history_size);
}

static int undo_history_add_item(UNDO_HISTORY *history, 
								 void *mem, size_t size) {
	UNDO_HISTORY_ITEM *new_item;

	new_item = realloc(history->item, 
					  sizeof(UNDO_HISTORY_ITEM) * (history->length + 1));
	if(new_item == NULL) {
		UNDO_ERROR(UNDO_NOMEM);
	}
	history->item = new_item;

	history->length++;

	history->ix = history->length - 1;
	history->item[history->ix].mem = mem;
	history->item[history->ix].size = size;

	UNDO_SUCCESS;
}

static UNDO_MEMORY_STREAM *undo_history_stream(UNDO_HISTORY *history) {
	UNDO_MEMORY_STREAM *stream;

	stream = NEW(UNDO_MEMORY_STREAM);
	if(stream == NULL)
		UNDO_ERROR_NULL(UNDO_NOMEM);

	stream->implementation = history;
	stream->destroy = undo_history_stream_destroy;
	stream->read = undo_history_stream_read;

	return stream;
}

static void undo_history_shrink(UNDO_HISTORY *history, size_t size) {
	int first_saved;
	int ix;
	size_t total;

	total = 0;
	first_saved = 0;
	for(ix = 0; ix < history->length; ix++) {
		total += history->item[ix].size;
		while(total > size) {
			total -= history->item[first_saved].size;
			first_saved++;
		}
	}

	undo_history_strip(history, first_saved);
}

static void undo_history_strip(UNDO_HISTORY *history, int count) {
	int ix;

	if(count == 0)
		return;

	for(ix = 0; ix < count; ix++) {
		free(history->item[ix].mem);
	}

	memmove(history->item, history->item + count, 
			sizeof(UNDO_HISTORY_ITEM) * (history->length - count));
	history->length -= count;
	history->ix = history->length - 1;
}

static void undo_history_strip_trailing(UNDO_HISTORY *history) {
	int ix;
	UNDO_HISTORY_ITEM *new_item;
    
	for(ix = history->ix + 1; ix < history->length; ix++) {
		free(history->item[ix].mem);
	}

	if(history->length > history->ix + 1) {
		new_item = realloc(history->item, 
			sizeof(UNDO_HISTORY_ITEM) * (history->ix + 1));
		if(new_item == NULL) {
			return;
		}
		history->item = new_item;

		history->length = history->ix + 1;
	}
}
