#!/bin/bash
#
# run as incremental.tpcas-1st.stage.2.tpcas-1st.stage.com <list of subdir> <FULL path tpcas-1st.stage dir> <FULL path tpcas-2nd.stage dir>  <path of tmp dir>
#
TMPDIR="/tmp"
if [ ${#4} -ne 0 ]
then
    TMPDIR=$4
fi
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
for i in $(cat "$1");
do
    mkdir -p $3/$i
    ln -s $2/$i/images $3/$i/.
    for j in $2/$i/*gz
    do
	if [ -f $j ]
	then
	    DESTLINK=$(echo $j | sed s#$2/##g | sed s/\(/_OB_/g | sed s/\)/_CB_/g | sed 's/\//_SLASH_/g')
	    ln -s $j $TMPFILENAME1/$DESTLINK
	    gunzip -f $TMPFILENAME1/$DESTLINK
	fi
    done
done
/usr/local/bin/runAECpp /usr/local/uima_descriptors/TpLexiconAnnotatorFromPg.xml -xmi $TMPFILENAME1/ $TMPFILENAME2/ 
for i in $TMPFILENAME2/*.tpcas
do
    gzip $i
done
for i in $TMPFILENAME2/*.gz
do
    DEST=$(echo $i | sed s/_OB_/\(/g | sed s/_CB_/\)/g | sed 's/_SLASH_/\//g' | sed s#$TMPFILENAME2#$3#g)
    mv $i $DEST
done
rm -rf $TMPFILENAME1/ $TMPFILENAME2/
