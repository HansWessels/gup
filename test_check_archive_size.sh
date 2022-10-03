#! /bin/sh
#
# utility script for `make test`
# checks the size (in bytes) of the arj file produced.

IST=$( stat -L -c '%s' $1 )
SOLL=1010776

if [ $IST -eq $SOLL ] ; then
	cat <<EOF


>>> HOERA! $SOLL bytes expected and produced. <<<

EOF
	exit 0
else
	cat <<EOF


### FAIL! $SOLL bytes expected, $IST bytes in current ARJ archive. ###

EOF
	exit 1
fi

