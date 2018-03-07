#!/bin/bash
#
# This script has been compiled from a history file and still needs to be tested.
#
PDF="/mnt/data1/textpresso/tcpipelines/pdfs2secondstage/pdfs-classified/"
TPCAS1="/mnt/data1/textpresso/tcpipelines/pdfs2secondstage/tpcas-1st.stage-classified/"
DIRLISTS="/usr/local/textpresso/etc/dirlists/pdf"
IFS=$(echo -en "\n\b")
cd $PDF
for i in *
do 
    find "$i" -maxdepth 1 | grep '/' > "$DIRLISTS/$i"
done
cd $TPCAS1
for i in $(ls $DIRLISTS)
do 
    echo /usr/local/textpresso/sources/SupportingScripts/pipelines/incremental.pdf.2.tpcas-1st.stage.com \"$DIRLISTS/$i\" $PDF | batch
done
