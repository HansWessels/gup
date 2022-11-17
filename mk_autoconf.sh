#! /bin/sh
#
# regenerate configure script

make superclean

rm config.*
autoconf
automake --add-missing
autoheader

./configure

make test-minimum
