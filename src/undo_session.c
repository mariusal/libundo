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

#include "undo.h"
#include "undo_private.h"
#include "undo_memory.h"

UNDO *undo_new(const char *session_name) {
	UNDO *undo = NULL;

	if(session_name == NULL)
		UNDO_ERROR_NULL(UNDO_BADPARAM);

	undo = NEW(UNDO);
	if(undo == NULL)
		UNDO_ERROR_NULL(UNDO_NOMEM);

	undo->name = (char *)malloc(strlen(session_name) + 1);
	if(undo->name == NULL) {
		undo_destroy(undo);
		UNDO_ERROR_NULL(UNDO_NOMEM);
	}
	strcpy(undo->name, session_name);

	undo->memory = undo_memory_new();
	if(undo->memory == NULL) {
		undo_destroy(undo);
		UNDO_ERROR_NULL(UNDO_NOMEM);
	}

	undo->history = undo_history_new();
	if(undo->history == NULL) {
		undo_destroy(undo);
		UNDO_ERROR_NULL(UNDO_NOMEM);
	}

	return undo;
}

int undo_destroy(UNDO *undo) {
	if(undo == NULL)
		UNDO_ERROR(UNDO_NOSESSION);

	if(undo->name)
		free(undo->name);

	if(undo->memory)
		undo_memory_destroy(undo->memory);
	
	if(undo->history)
		undo_history_destroy(undo->history);

	free(undo);

	UNDO_SUCCESS;
}

int undo_set_memory_limit(UNDO *undo, size_t max_memory) {
	if(undo == NULL)
		UNDO_ERROR(UNDO_NOSESSION);

	undo_history_set_memory_limit(undo->history, max_memory);

	UNDO_SUCCESS;
}

int undo_set_history_logical(UNDO *undo, int onoff) {
	if(undo == NULL)
		UNDO_ERROR(UNDO_NOSESSION);

	undo_history_set_logical(undo->history, onoff);

	UNDO_SUCCESS;
}

unsigned undo_get_undo_count(const UNDO *undo) {
	if(undo == NULL)
		return 0;

	return undo_history_undo_count(undo->history);
}

unsigned undo_get_redo_count(const UNDO *undo) {
	if(undo == NULL)
		return 0;

	return undo_history_redo_count(undo->history);
}

int undo_undo(UNDO *undo) {
	UNDO_MEMORY_STREAM *stream;
	int ret;

	if(undo == NULL)
		UNDO_ERROR(UNDO_NOSESSION);

	if(undo_history_undo_count(undo->history) == 0)
		UNDO_ERROR(UNDO_NODO);

	stream = undo_history_undo(undo->history);
	if(stream == NULL)
		UNDO_ERROR(UNDO_NOMEM);
	ret = undo_memory_set(undo->memory, stream);
	stream->destroy(stream);

	return ret;
}

int undo_redo(UNDO *undo) {
	UNDO_MEMORY_STREAM *stream;
	int ret;

	if(undo == NULL)
		UNDO_ERROR(UNDO_NOSESSION);
	
	if(undo_history_redo_count(undo->history) == 0)
		UNDO_ERROR(UNDO_NODO);

	stream = undo_history_redo(undo->history);
	if(stream == NULL)
		UNDO_ERROR(UNDO_NOMEM);
	ret = undo_memory_set(undo->memory, stream);
	stream->destroy(stream);

	return ret;
}

int undo_snapshot(UNDO *undo) {
	UNDO_MEMORY_STREAM *stream;
	int ret;

	if(undo == NULL)
		UNDO_ERROR(UNDO_NOSESSION);

	stream = undo_memory_stream(undo->memory);
	if(stream == NULL)
		UNDO_ERROR(UNDO_NOMEM);
	ret = undo_history_record(undo->history, stream);
	stream->destroy(stream);

	return ret;
}

void *undo_malloc(UNDO *undo, size_t size) {
	if(undo == NULL)
		UNDO_ERROR_NULL(UNDO_NOSESSION);

	return undo_memory_alloc(undo->memory, size);
}

void *undo_realloc(UNDO *undo, void *mem, size_t size) {
	size_t min_size;
	void *new_mem;

	if(undo == NULL)
		UNDO_ERROR_NULL(UNDO_NOSESSION);

	if(mem == NULL)
		return undo_memory_alloc(undo->memory, size);

	min_size = undo_memory_size(undo->memory, mem);
	if(size < min_size)
		min_size = size;

	if(size == min_size)
		return mem;

	new_mem = undo_memory_alloc(undo->memory, size);
	if(new_mem == NULL)
		UNDO_ERROR_NULL(UNDO_NOMEM);

	memcpy(new_mem, mem, min_size);
	undo_memory_free(undo->memory, mem);
	
	return new_mem; 
}

void undo_free(UNDO *undo, void *mem) {
	if(undo == NULL)
		return;

	undo_memory_free(undo->memory, mem);
}
