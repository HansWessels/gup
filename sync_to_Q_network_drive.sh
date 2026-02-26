#!/usr/bin/env bash

rm -rf .git
# cp -r ./* ~/q/nyh/gup/
#
# cat config.guess > ~/q/nyh/gup/config.guess
# cat config.sub   > ~/q/nyh/gup/config.sub

# rsync -r -I -U -t --copy-links -i -c --ignore-existing --stats --progress '--exclude=.git/' '--exclude=*.[ao]' '--exclude=*.l[ao]' '--exclude=*.bak' '--exclude=*.i' '--exclude=*~' '--exclude=autom4te.cache/*'   . ~/q/nyh/Atari/e/pc/gup/
rsync -a -v -W --copy-links -c --stats --progress '--exclude=.git/' '--exclude=*.[ao]' '--exclude=*.l[ao]' '--exclude=*.Po' '--exclude=*.bak' '--exclude=*.i' '--exclude=*~' '--exclude=autom4te.cache/*' '--exclude=*.lock*' '--exclude=*~lock*' '--exclude=*.arj' '--exclude=*.pdb' '--exclude=*.*log'  . ~/q/nyh/Atari/e/pc/gup/
