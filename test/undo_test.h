#ifndef __undo_test_h
#define __undo_test_h

#include <assert.h>

#include "undoP.h"

UNDO_MEMORY_STREAM *undo_test_memory_stream(void *mem, unsigned size);

#endif
