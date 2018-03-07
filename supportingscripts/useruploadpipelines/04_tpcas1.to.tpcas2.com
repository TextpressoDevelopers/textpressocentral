#!/bin/bash
#
# 
# run with following command (to avoid collisions)
# flock -n /tmp/04_tpcas1.to.tpcas2.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/04_tpcas1.to.tpcas2.com
#
TMPDIR="/tmp"
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
DATE=$(date +%Y%m%d%H%M%S);
IFS=$(echo -en "\n\b")
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
for source in $(find /usr/local/textpresso/useruploads -name "*.gz" | grep tpcas-1)
do
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
    fi
done
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
chown -R www-data:www-data /usr/local/textpresso/useruploads
rm -f /tmp/04_tpcas1.to.tpcas2.lock
