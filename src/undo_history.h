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

#ifndef __undo_history_h
#define __undo_history_h

#include <stdlib.h>

#include "undo_memory_stream.h"

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

#endif
