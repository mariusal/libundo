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

static UNDO *undo_session = NULL;

int undo_new(char *session_name) {
	if(session_name == NULL)
		UNDO_ERROR(UNDO_BADPARAM);

	undo_session = NEW(UNDO);
	if(undo_session == NULL)
		UNDO_ERROR(UNDO_NOMEM);

	undo_session->name = (char *)malloc(strlen(session_name) + 1);
	if(undo_session->name == NULL) {
		undo_destroy();
		UNDO_ERROR(UNDO_NOMEM);
	}
	strcpy(undo_session->name, session_name);

	undo_session->memory = undo_memory_new();
	if(undo_session->memory == NULL) {
		undo_destroy();
		UNDO_ERROR(UNDO_NOMEM);
	}

	undo_session->history = undo_history_new();
	if(undo_session->history == NULL) {
		undo_destroy();
		UNDO_ERROR(UNDO_NOMEM);
	}

	UNDO_SUCCESS;
}

int undo_destroy(void) {
	if(undo_session == NULL)
		UNDO_ERROR(UNDO_NOSESSION);

	if(undo_session->name)
		free(undo_session->name);

	if(undo_session->memory)
		undo_memory_destroy(undo_session->memory);
	
	if(undo_session->history)
		undo_history_destroy(undo_session->history);

	undo_session = NULL;

	UNDO_SUCCESS;
}

UNDO *undo_get_session(void) {
	return undo_session;
}

int undo_set_session(UNDO *undo) {
	undo_session = undo;

	UNDO_SUCCESS;
}

int undo_set_memory_limit(size_t max_memory) {
	if(undo_session == NULL)
		UNDO_ERROR(UNDO_NOSESSION);

	undo_history_set_memory_limit(undo_session->history, max_memory);

	UNDO_SUCCESS;
}

unsigned undo_get_undo_count(void) {
	if(undo_session == NULL)
		return 0;

	return undo_history_undo_count(undo_session->history);
}

unsigned undo_get_redo_count(void) {
	if(undo_session == NULL)
		return 0;

	return undo_history_redo_count(undo_session->history);
}

int undo_undo(void) {
	UNDO_MEMORY_STREAM *stream;
	int ret;

	if(undo_session == NULL)
		UNDO_ERROR(UNDO_NOSESSION);

	if(undo_history_undo_count(undo_session->history) == 0)
		UNDO_ERROR(UNDO_NODO);

	stream = undo_history_undo(undo_session->history);
	if(stream == NULL)
		UNDO_ERROR(UNDO_NOMEM);
	ret = undo_memory_set(undo_session->memory, stream);
	stream->destroy(stream);

	return ret;
}

int undo_redo(void) {
	UNDO_MEMORY_STREAM *stream;
	int ret;

	if(undo_session == NULL)
		UNDO_ERROR(UNDO_NOSESSION);
	
	if(undo_history_redo_count(undo_session->history) == 0)
		UNDO_ERROR(UNDO_NODO);

	stream = undo_history_redo(undo_session->history);
	if(stream == NULL)
		UNDO_ERROR(UNDO_NOMEM);
	ret = undo_memory_set(undo_session->memory, stream);
	stream->destroy(stream);

	return ret;
}

int undo_snapshot(void) {
	UNDO_MEMORY_STREAM *stream;
	int ret;

	if(undo_session == NULL)
		UNDO_ERROR(UNDO_NOSESSION);

	stream = undo_memory_stream(undo_session->memory);
	if(stream == NULL)
		UNDO_ERROR(UNDO_NOMEM);
	ret = undo_history_record(undo_session->history, stream);
	stream->destroy(stream);

	return ret;
}

void *undo_malloc(size_t size) {
	if(undo_session == NULL)
		UNDO_ERROR_NULL(UNDO_NOSESSION);

	return undo_memory_alloc(undo_session->memory, size);
}

void *undo_realloc(void *mem, size_t size) {
	size_t min_size;
	void *new_mem;

	if(undo_session == NULL)
		UNDO_ERROR_NULL(UNDO_NOSESSION);

	if(mem == NULL)
		return undo_memory_alloc(undo_session->memory, size);

	min_size = undo_memory_size(undo_session->memory, mem);
	if(size < min_size)
		min_size = size;

	if(size == min_size)
		return mem;

	new_mem = undo_memory_alloc(undo_session->memory, size);
	if(new_mem == NULL)
		UNDO_ERROR_NULL(UNDO_NOMEM);

	memcpy(new_mem, mem, min_size);
	undo_memory_free(undo_session->memory, mem);
	
	return new_mem; 
}

void undo_free(void *mem) {
	if(undo_session == NULL)
		return;

	undo_memory_free(undo_session->memory, mem);
}
