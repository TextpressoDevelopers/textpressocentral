#!/bin/bash
#
# 
# run with following command (to avoid collisions)
# flock -n /tmp/tpcas1.2.tpcas2.4.1.literature.lock -c ./04_tpcas1.2.tpcas2.4.1.literature.com
#
TMPDIR="/tmp"
LIT="$1"
SOURCEDIR="/home/textpresso/tcpipelines/permanent/tpcas-1/$LIT"
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
DATE=$(date +%Y%m%d%H%M%S)
IFS=$(echo -en "\n\b")
BATCHSIZE=10000
until [ ! -d $TMPFILENAME1 ]
do
    TMPFILENAME1="$TMPDIR/"$(echo $RANDOM$DATE$RANDOM | md5sum | cut -f 1 -d " ");
done
mkdir -p $TMPFILENAME1 || exit 1
until [ ! -d $TMPFILENAME2 ]
do
    TMPFILENAME2="$TMPDIR/"$(echo $RANDOM$DATE$RANDOM | md5sum | cut -f 1 -d " ");
done
mkdir -p $TMPFILENAME2 || exit 1
k=0
for source in $(find "$SOURCEDIR" -name "*.gz" | grep tpcas-1)
do
    # copy $source to $TMPFILENAME1 dir
    destination=$(echo $source | sed s/tpcas-1/tpcas-2/)
    if [ $source -nt $destination ];
    then
	destfile=$(echo $destination | sed s/\(/_OB_/g | sed s/\)/_CB_/g | sed 's/\//_SLASH_/g')
	cp $source $TMPFILENAME1/$destfile
	gunzip $TMPFILENAME1/$destfile
	destdir=$(dirname $source | sed s/tpcas-1/tpcas-2/)
	mkdir -p $destdir
	if [ -d $(dirname $source)/images ];
	then
	    ln -s $(dirname $source)/images $destdir/.
	fi
	((k += 1)) # transfer of one file complete, increase counter
    fi
    if [ "$k" -eq "$BATCHSIZE" ]
    then # this batch is full, start processing
	if [ "$(ls -A $TMPFILENAME1)" ];
	then
	    /usr/local/bin/runAECpp /usr/local/uima_descriptors/TpLexiconAnnotatorFromPg.xml -xmi $TMPFILENAME1/ $TMPFILENAME2/
	    for i in $TMPFILENAME2/*.tpcas
	    do
		gzip $i
	    done
	    for i in $TMPFILENAME2/*.gz
	    do
		DEST=$(echo $i | sed s/_OB_/\(/g | sed s/_CB_/\)/g | sed 's/_SLASH_/\//g' | sed s#$TMPFILENAME2##g)
	        mv $i $DEST
	    done
	fi
	rm -rf $TMPFILENAME1/ $TMPFILENAME2/ # remove old $TMPFILENAME1 and $TMPFILENAME2, make new ones 
	until [ ! -d $TMPFILENAME1 ]
	do
	    TMPFILENAME1="$TMPDIR/"$(echo $RANDOM$DATE$RANDOM | md5sum | cut -f 1 -d " ");
	done
	mkdir -p $TMPFILENAME1 || exit 1
	until [ ! -d $TMPFILENAME2 ]
	do
	    TMPFILENAME2="$TMPDIR/"$(echo $RANDOM$DATE$RANDOM | md5sum | cut -f 1 -d " ");
	done
	mkdir -p $TMPFILENAME2 || exit 1
	k=0 # reset batchsize counter
    fi
done
# process remaing batch
if [ "$(ls -A $TMPFILENAME1)" ];
then
    /usr/local/bin/runAECpp /usr/local/uima_descriptors/TpLexiconAnnotatorFromPg.xml -xmi $TMPFILENAME1/ $TMPFILENAME2/
    for i in $TMPFILENAME2/*.tpcas
    do
	gzip $i
    done
    for i in $TMPFILENAME2/*.gz
    do
	DEST=$(echo $i | sed s/_OB_/\(/g | sed s/_CB_/\)/g | sed 's/_SLASH_/\//g' | sed s#$TMPFILENAME2##g)
	mv $i $DEST
    done
fi
rm -rf $TMPFILENAME1/ $TMPFILENAME2/
rm -f /tmp/tpcas1.2.tpcas2.4.1.literature.lock
