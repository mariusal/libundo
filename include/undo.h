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

#ifndef __libundo_h
#define __libundo_h

#include <stdlib.h>  /* For size_t */

/* Versioning */
#define UNDO_MAJOR_VERSION  0
#define UNDO_MINOR_VERSION  8
#define UNDO_REVISION       1

void undo_get_version(unsigned *major, unsigned *minor, unsigned *revision);

typedef struct _UNDO UNDO;

/*  Undo session management  */
UNDO *undo_new(void);
int undo_destroy(UNDO *undo);

/*  Undo limits  */
int undo_set_memory_limit(UNDO *undo, size_t max_memory);

/*  History type  */
int undo_set_history_logical(UNDO *undo, int onoff);

/*  Memory management for undo-watched memory  */
void *undo_malloc(UNDO *undo, size_t size);
void *undo_realloc(UNDO *undo, void *mem, size_t size);
void undo_free(UNDO *undo, void *mem);

/*  Recording functions  */
int undo_snapshot(UNDO *undo);

/*  Undo and redo  */
int undo_undo(UNDO *undo);
int undo_redo(UNDO *undo);

unsigned undo_get_undo_count(const UNDO *undo);
unsigned undo_get_redo_count(const UNDO *undo);

/*  Error codes  */
#define UNDO_NOERROR        0   /* No error */
#define UNDO_BADPARAM       1   /* Bad parameter passed to function */
#define UNDO_NOMEM          2   /* Not enough memory */
#define UNDO_NOSESSION      3   /* No undo session */
#define UNDO_NODO           4   /* Nothing to undo/redo */
#define UNDO_NOLIMIT        5   /* No memory limit specified */

/*  Error recovery  */
int undo_get_errcode(UNDO *undo);
char *undo_strerror(int errcode);

#endif
