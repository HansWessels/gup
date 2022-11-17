#! /bin/bash
#

cat <<EOF

>>> Pushing our work to both remotes...

EOF
for f in GerHobbelt HansWessels ; do 
	echo $f 
	git push --all git@github.com:$f/gup.git
done

# fetch all remote work as an afterthought
cat <<EOF


>>> Fetching all remote work...

EOF
git pull --all 
