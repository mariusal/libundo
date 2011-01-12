ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = ChangeLog
SUBDIRS = doc include src test

changelog :
	cvs2cl.pl

ChangeLog : changelog
