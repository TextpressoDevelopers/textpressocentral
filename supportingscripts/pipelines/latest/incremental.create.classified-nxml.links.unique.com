#!/bin/bash
# 
# $1 is the name of the directory in which all dirs and links are created.
# $2 is the prefix one should add to all links
# file names in /usr/local/textpresso/tmp/allnxml.subject.txt has to be relative,
# i.e., of the form subdir/abc.nxml
#
####
IFS=$(echo -en "\n\b")
wc -l /usr/local/textpresso/tmp/allnxml.subject.txt; /usr/local/textpresso/sources/supportingscripts/pipelines/housekeeping.collect.subjects.com; wc -l /usr/local/textpresso/etc/collection/*/*
for i in $(ls /usr/local/textpresso/etc/collection/unique)
do 
    mkdir -p "$1/$i"
done
for i in $(ls /usr/local/textpresso/etc/collection/unique)
do 
    for j in $(cut -f 1 "/usr/local/textpresso/etc/collection/unique/$i" | cut -f 2 -d ":" | cut -f 1 -d '/')
    do 
	ln -s  $2/$j "$1/$i/" 2>/dev/null
    done
done
