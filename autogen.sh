#!/bin/sh
#autoreconf --force --install -I config -I m4

# Make m4 directory if it doesn't exist.
if [ ! -d "m4" ]; then
	mkdir "m4"
fi

echo "- libtoolize"
libtoolize --force

echo "- aclocal"
aclocal

echo "- autoconf"
autoconf

echo "- autoheader"
autoheader

echo "- automake"
automake --add-missing

./configure $@
