INCLUDES = -I${top_srcdir}/include

TESTS = test1$(EXEEXT) \
	test2$(EXEEXT) \
	test3$(EXEEXT) \
	test4$(EXEEXT) \
	test5$(EXEEXT) \
	test6$(EXEEXT) \
	test7$(EXEEXT) \
	test8$(EXEEXT) \
	test9$(EXEEXT) \
	test10$(EXEEXT) \
	test11$(EXEEXT) \
	test12$(EXEEXT)

noinst_LIBRARIES = libundo_test.a
libundo_test_a_SOURCES = undo_test.h undo_test.c

check_PROGRAMS = $(TESTS)

AM_LDFLAGS = libundo_test.a ${top_srcdir}/src/libundo.la
