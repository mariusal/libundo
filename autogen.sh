#!/bin/sh

libtoolize --copy --automake --force					&& \
aclocal -I .								&& \
autoconf								&& \
automake --include-deps --add-missing --copy --foreign --no-force	&& \
./configure "$@"
