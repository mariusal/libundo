INCLUDES = -I${top_srcdir}/include

lib_LTLIBRARIES = libundo.la

libundo_la_LDFLAGS = -version-info @UNDO_CURRENT@:@UNDO_REVISION@:@UNDO_AGE@

libundo_la_SOURCES = \
    undo_error.c \
    undo_history.c \
    undo_session.c \
    undo_memory_stream.c \
    undo_memory.c \
    undo_version.c
