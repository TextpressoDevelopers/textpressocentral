#!/bin/bash
#
# run with following command (to avoid collisions)
# flock -n /tmp/03_nxmltar2tpcas-1.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/03_nxmltar2tpcas-1.com
#
#
TMPDIR="/tmp"
DATE=$(date +%Y%m%d%H%M%S);
base="/usr/local/textpresso/useruploads"
IFS=$(echo -en "\n\b")
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
for user in $(ls $base)
do
    for literature in $(ls $base/$user/literatures)
    do
	for nxmltardir in $(ls $base/$user/literatures/$literature/nxml-tar)
	do
	    # have to repeat $literature in tgtdir path to comply with downstream pipeline
	    tgtdir="$base/$user/literatures/$literature/tpcas-1/$literature/$nxmltardir"
	    for nxmltar in $(ls $base/$user/literatures/$literature/nxml-tar/$nxmltardir | grep .gz)
	    do
		until [ ! -d $TMPFILENAME ]
		do
		    TMPFILENAME="$TMPDIR/"$(echo $RANDOM$DATE$RANDOM | md5sum | cut -f 1 -d " ");
		done
		mkdir -p $TMPFILENAME
		srcfile="$base/$user/literatures/$literature/nxml-tar/$nxmltardir/$nxmltar"
		tar xfz $srcfile -C $TMPFILENAME
		for i in $(ls $TMPFILENAME/)
		do
		    for j in $(ls $TMPFILENAME/$i/ | grep .nxml)
		    do
			filename=$(basename "$j")
			filename="${filename%.*}"
			tgtfile="$tgtdir/$filename.tpcas"
			tgtfilegz="$tgtdir/$filename.tpcas.gz"
			mkdir -p $tgtdir
			mkdir -p "$tgtdir/images"
			if [ $srcfile -nt $tgtfilegz ];
			then
			    # leave cd and second argument of xml2tpcas this way to get filename in tpcas file right
			    cd $base/$user/literatures/$literature/tpcas-1
			    /usr/local/textpresso/bin/xml2tpcas $TMPFILENAME/$i/$j $literature/$nxmltardir
			    gzip $tgtfile
			    mv $TMPFILENAME/$i/*.jpg $tgtdir/images
			    mv $TMPFILENAME/$i/*.jpeg $tgtdir/images
			    mv $TMPFILENAME/$i/*.gif $tgtdir/images
			fi
		    done
		done
		rm -rf $TMPFILENAME
	    done
	done
	chown -R www-data:www-data $base/$user/literatures/$literature/tpcas-1/$literature
    done
done
rm -f /tmp/03_nxmltar2tpcas-1.lock
