#! /bin/sh
#
# regenerate configure script

rm config.*
autoconf
automake --add-missing
