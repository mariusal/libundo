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

#ifndef __undo_error_h
#define __undo_error_h

#include "undo.h"

#define NEW(type) ((type *)calloc(1, sizeof(type)))

#define UNDO_SUCCESS               return UNDO_NOERROR;
#define UNDO_ERROR(undo, err)      return (undo_set_error(undo, err), (err));
#define UNDO_ERROR_NULL(undo, err) return (undo_set_error(undo, err), NULL);

void undo_set_error(UNDO *undo, int err);

#endif
