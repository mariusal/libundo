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

#include "undoP.h"

void undo_set_error(UNDO *undo, int err) {
	if (undo) {
        	undo->last_error = err;
	}
}

int undo_get_errcode(UNDO *undo) {
	int ret;

	if (undo) {
		ret = undo->last_error;
		undo->last_error = UNDO_NOERROR;
		return ret;
	} else {
		return UNDO_NOSESSION;
	}
}

char *undo_strerror(int errcode) {
	char *err_string[] = {
		"No error",
		"Bad parameter passed to undo function",
		"Out of memory",
		"No active undo session",
		"Nothing to undo/redo",
		"No undoable memory limit set"
	};

	if(errcode < 0)
		return NULL;

	if(errcode >= sizeof(err_string) / sizeof(char *))
		return NULL;

	return err_string[errcode];
}
