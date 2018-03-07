#!/bin/bash
#
# The list of articles to be extracted is in $1
# column 2 of $1 is extracted
####
DESTDIR=/home/textpresso/tcpipelines/permanent/nxmls
###
for i in $(cut -f 2 $1)
do
    tar xfz $i -C $DESTDIR
    rm $DESTDIR/*/*.pdf 2>/dev/null
    gzip $DESTDIR/*/*.nxml
done
for i in $DESTDIR/*
do 
    mkdir -p $i/images
    for j in $(ls $i/* | grep -v nxml.gz | grep -v $i/images)
    do
	mv $j $i/images/
    done
done
