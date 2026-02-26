#!/usr/bin/env bash

# cp -r ~/q/nyh/Atari/e/pc/gup/ ../ ; rm -rf .git
rsync -v -r --exclude=.git/ '--exclude=test/Mame/*' '--exclude=*.[ao]' '--exclude=*.l[ao]' '--exclude=*.bak' '--exclude=*.i' '--exclude=*.obj' '--exclude=*~' '--exclude=autom4te.cache/*' '--exclude=.vs' '--exclude=*.pdb' '--exclude=*.*log' -u -t --copy-links   ~/q/nyh/Atari/e/pc/gup/ ./
