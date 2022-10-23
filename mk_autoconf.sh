#! /bin/sh
#
# regenerate configure script

make superclean

rm config.*
autoconf
automake --add-missing

./configure

make test
