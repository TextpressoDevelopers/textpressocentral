#!/bin/bash
#
# run with following command (to avoid collisions)
# flock -n /tmp/02_nxml2tpcas-1.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/02_nxml2tpcas-1.com
#
#
base="/usr/local/textpresso/useruploads"
IFS=$(echo -en "\n\b")
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
for user in $(ls $base)
do
    for literature in $(ls $base/$user/literatures)
    do
	for nxmldir in $(ls $base/$user/literatures/$literature/nxml)
	do
	    # have to repeat $literature in tgtdir path to comply with downstream pipeline
	    tgtdir="$base/$user/literatures/$literature/tpcas-1/$literature/$nxmldir"
	    for nxml in $(ls $base/$user/literatures/$literature/nxml/$nxmldir | grep .nxml)
	    do
		filename=$(basename "$nxml")
		filename="${filename%.*}"
		srcfile="$base/$user/literatures/$literature/nxml/$nxmldir/$filename.nxml"
		tgtfile="$tgtdir/$filename.tpcas"
		tgtfilegz="$tgtdir/$filename.tpcas.gz"
		mkdir -p $tgtdir
		if [ $srcfile -nt $tgtfilegz ];
		then
		    # leave cd and second argument of xml2tpcas this way to get filename in tpcas file right
		    cd $base/$user/literatures/$literature/tpcas-1
		    /usr/local/textpresso/bin/xml2tpcas $srcfile $literature/$nxmldir
		    gzip $tgtfile
		fi
	    done       
	done
	chown -R www-data:www-data $base/$user/literatures/$literature/tpcas-1/$literature
    done
done
rm -f /tmp/02_nxml2tpcas-1.lock
