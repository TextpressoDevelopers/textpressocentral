#!/bin/bash
# 
# $1 is the name of the directory in which all subdirs are located. Script only checks for *.tpcas.gz files
#
####
IFS=$(echo -en "\n\b")
cd $1
for i in $(find -L . -name "*.tpcas.gz")
do
    mdnew=$(md5sum $i | cut -f 1 -d " ")
    mdold=$(cat ${i%.tpcas.gz}.md5sum 2>/dev/null)
    if [ "$mdnew" != "$mdold" ]
    then
	echo $i
    fi
    echo $mdnew > ${i%.tpcas.gz}.md5sum
done
