#!/bin/bash
#
# runs as incremental.nxml.2.tpcas-1st.stage.com <list of subdir> <nxml dir>
#
# The <tpcas 1st stage dir> needs to be the current work directory
#
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
IFS=$(echo -en "\n\b")
for i in $(cat "$1");
do
    mkdir -p $i
    for j in $(find -L "$2/$i/" -name "*nxml.gz")
    do
	gunzip "$j"
    done
    for j in $(find -L "$2/$i/" -name "*.nxml")
    do
	/usr/local/textpresso/bin/xml2tpcas $j "$i"
    done
    for j in $(find -L "$i" -name "*.tpcas")
    do
	gzip "$j"
    done
    for j in $(find -L  "$2/$i/" -name "*.nxml")
    do
	gzip "$j"
    done
done
