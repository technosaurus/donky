#!/bin/sh
#autoreconf --force --install -I config -I m4

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
