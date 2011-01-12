INCLUDES = -I${top_srcdir}/include

lib_LTLIBRARIES = libundo.la

libundo_la_LDFLAGS = -no-undefined \
	-version-info @UNDO_CURRENT@:@UNDO_REVISION@:@UNDO_AGE@

libundo_la_SOURCES = \
    undo_error.c \
    undo_history.c \
    undo_session.c \
    undo_memory_stream.c \
    undo_memory.c \
    undo_version.c

libundo_la_CFLAGSS = -Wall -O2

$(libundo_la_OBJECTS) : ${top_srcdir}/include/undoP.h \
	${top_srcdir}/include/undo.h
