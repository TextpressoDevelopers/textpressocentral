#!/bin/bash
#
# This script has been compiled from a history file and still needs to be tested.
#
NXML="/mnt/data1/textpresso/tcpipelines/pmc2secondstage/nxmls-classified/"
TPCAS1="/mnt/data1/textpresso/tcpipelines/pmc2secondstage/tpcas-1st.stage-classified/"
DIRLISTS="/usr/local/textpresso/etc/dirlists/nxml"
IFS=$(echo -en "\n\b")
cd $NXML
for i in *
do 
    find "$i" -maxdepth 1 | grep '/' > "$DIRLISTS/$i"
done
cd $TPCAS1
for i in $(ls $DIRLISTS)
do 
    echo /usr/local/textpresso/sources/SupportingScripts/pipelines/incremental.nxml.2.tpcas-1st.stage.com \"$DIRLISTS/$i\" $NXML | batch
done
