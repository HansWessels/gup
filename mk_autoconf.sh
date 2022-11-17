#! /bin/sh
#
# regenerate configure script

make distclean superclean 

rm config.*
rm libtool

autoconf
automake --add-missing
autoheader

./configure

make test-minimum
