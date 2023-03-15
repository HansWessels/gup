#! /bin/sh
#
# regenerate configure script

make distclean superclean 

rm config.*
rm libtool

autoconf
# automake --add-missing
automake --add-missing --copy --force-missing
autoheader

# fix for git + Windows: ditch the symbolic links that may have been produced during the process above:
mkdir tmp
rsync --archive --copy-links --copy-dirlinks [a-z]* tmp/
rm -f [a-z]*
cp tmp/* .
rm -rf tmp/

./configure

make test-minimum
