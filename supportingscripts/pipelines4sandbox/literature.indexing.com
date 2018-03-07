#!/bin/bash
#
# 
# run with following command (to avoid collisions)
# flock -n /tmp/literature.indexing.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/literature.indexing.com
#
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
IFS=$(echo -en "\n\b")
LIT="$1"
BATCHSIZE=40000
LUCENEBASEDIR="/home/textpresso/tcpipelines/permanent/luceneindex"
TPCAS2BASEDIR="/home/textpresso/tcpipelines/permanent/tpcas-2"
INDEXTIMESTAMP="$TPCAS2BASEDIR/$LIT/lucenetimestamp"
unset flag;
for source in $(find "$TPCAS2BASEDIR/$LIT" -name "*.gz")
do
    if [ $source -nt $INDEXTIMESTAMP ];
    then
	flag=1
    fi
done
if [ "$flag" ];
then
    cd $TPCAS2BASEDIR
    # First deal with bib files.
    /usr/local/textpresso/bin/getbib $LIT
    # do indexing in batches
    # make tmpdir for first batch
    TMPDIR="/tmp"
    DATE=$(date +%Y%m%d%H%M%S)
    until [ ! -d $TMPFILENAME ]
    do
	TMPFILENAME="$TMPDIR/"$(echo $RANDOM$DATE$RANDOM | md5sum | cut -f 1 -d " ");
    done
    mkdir -p $TMPFILENAME || exit 1
    k=0 # paper counter
    l=0 # index counter
    for sourcedir in $(ls "$TPCAS2BASEDIR/$LIT")
    do
	ln -s "$TPCAS2BASEDIR/$LIT/$sourcedir" $TMPFILENAME/.
	((k += 1)) # transfer of one file complete, increase counter
	if [ "$k" -eq "$BATCHSIZE" ]
	then # this batch is full, start processing
	    if [ "$(ls -A $TMPFILENAME)" ];
            then
		# remove old indices
		rm -rf "$LUCENEBASEDIR/$LIT""_$l"
		# index batch
		echo "$LUCENEBASEDIR/$LIT""_$l"
		/usr/local/textpresso/bin/cas2singleindex $TMPFILENAME "$LUCENEBASEDIR/$LIT""_$l" new
		rm -rf $TMPFILENAME/ # remove old $TMPFILENAME, make new ones
		until [ ! -d $TMPFILENAME ]
		do
		    TMPFILENAME="$TMPDIR/"$(echo $RANDOM$DATE$RANDOM | md5sum | cut -f 1 -d " ");
		done
		mkdir -p $TMPFILENAME || exit 1
		((l += 1))
	    fi
	    k=0 # reset batchsize counter
	fi
    done
    # process remaing batch
    if [ "$(ls -A $TMPFILENAME)" ];
    then
	# remove old indices
	rm -rf "$LUCENEBASEDIR/$LIT""_$l"
	# index batch
	echo "$LUCENEBASEDIR/$LIT""_$l"
	/usr/local/textpresso/bin/cas2singleindex $TMPFILENAME "$LUCENEBASEDIR/$LIT""_$l" new
    fi
    # indexing done, update timestamp
    touch $INDEXTIMESTAMP
fi
rm -f /tmp/literature.indexing.lock
