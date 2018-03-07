#!/bin/bash
#
# run with following command (to avoid collisions)                         
# flock -n /tmp/find.new.celegans.pdfs.and.copy.lock -c ./find.new.celegans.pdfs.and.copy.com
#

IFS=$(echo -en "\n\b")
incoming="/home/textpresso/tcpipelines/permanent/pdfs/rsync_from_textpresso_dev/C. elegans"
pdfdir="/home/textpresso/tcpipelines/permanent/pdfs/C. elegans"
#
## make new dirs in pdfdir if they don't exist
#
for i in $(ls "$incoming")
do
    base=$(basename "$i")
    dirname="${base%.*}"
    if [ ! -d "$pdfdir/$dirname" ]
	then
       mkdir "$pdfdir/$dirname"
    fi
done
#
##
#
#
## copy from incoming dir to pdfdir if pdfs are different or non-existent
#

for i in $(ls "$incoming")
do
    base=$(basename "$i")
    dirname="${base%.*}"
    if [ -n "$(diff -q --new-file $incoming/$base $pdfdir/$dirname/$base)" ]
        then
	cp $incoming/$base $pdfdir/$dirname/$base
    fi
done
rm -f /tmp/find.new.celegans.pdfs.and.copy.lock
