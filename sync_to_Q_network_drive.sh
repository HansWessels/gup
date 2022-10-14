#! /bin/bash

rm -rf .git
#cp -r ./* ~/q/nyh/gup/ 
#
#cat config.guess > ~/q/nyh/gup/config.guess
#cat config.sub   > ~/q/nyh/gup/config.sub

rsync -v -r --exclude=.git/ '--exclude=*.[ao]' '--exclude=*.l[ao]' '--exclude=*.bak' '--exclude=*.i' '--exclude=*~' '--exclude=autom4te.cache/*'   -u -t -U --copy-links  ~/q/nyh/Atari/e/pc/gup/

