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

size_t undo_memory_stream_write(size_t offset, void *mem, size_t size,
								size_t *pos, void *data, size_t length) {
	size_t begin_mem, end_mem;
	size_t begin_data;

	if(offset < *pos) {
		begin_mem = *pos - offset;
		begin_data = 0;
	} else {
		begin_mem = 0;
		begin_data = offset - *pos;
	}

	if(offset + size < *pos + length) {
		end_mem = size;
	} else {
		end_mem = size - (offset + size - *pos - length);
	}

	*pos += length;

	if((signed)end_mem < 0)
		return 0;
	if(end_mem <= begin_mem)
		return 0;

	memcpy(MEMORY_OFFSET(mem, begin_mem), 
		   MEMORY_OFFSET(data, begin_data),
		   end_mem - begin_mem);

	return end_mem - begin_mem;
}

size_t undo_memory_stream_length(UNDO_MEMORY_STREAM *stream) {
	size_t len, diff;
	char buff[4096];

	len = 0;
	do {
		diff = stream->read(stream, len, buff, 4096);
		len += diff;
	} while(diff != 0);

	return len;
}


size_t undo_memory_stream_read_header(UNDO_MEMORY_STREAM *stream, 
									size_t offset,
									UNDO_MEMORY_STREAM_BLOCK_HEADER *header) {
	size_t num_read;

	num_read = 0;
	num_read += stream->read(stream, offset + num_read, 
							 &header->addr, sizeof(void *));
	num_read += stream->read(stream, offset + num_read, 
							 &header->size, sizeof(size_t));
	num_read += stream->read(stream, offset + num_read, 
							 &header->flags, sizeof(unsigned));
	
	return num_read;
}

