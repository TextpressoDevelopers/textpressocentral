#!/bin/bash
#
# run with following command (to avoid collisions)
# flock -n /tmp/01_pdf2tpcas-1.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/01_pdf2tpcas-1.com
#
#
base="/usr/local/textpresso/useruploads"
IFS=$(echo -en "\n\b")
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
for user in $(ls $base)
do
    for literature in $(ls $base/$user/literatures)
    do
	for pdfdir in $(ls $base/$user/literatures/$literature/pdf)
	do
	    # have to repeat $literature in tgtdir path to comply with downstream pipeline
	    tgtdir="$base/$user/literatures/$literature/tpcas-1/$literature/$pdfdir"
	    for pdf in $(ls $base/$user/literatures/$literature/pdf/$pdfdir | grep .pdf)
	    do
		filename=$(basename "$pdf")
		filename="${filename%.*}"
		srcfile="$base/$user/literatures/$literature/pdf/$pdfdir/$filename.pdf"
		tgtfile="$tgtdir/$filename.tpcas"
		tgtfilegz="$tgtdir/$filename.tpcas.gz"
		mkdir -p $tgtdir
		mkdir -p "$tgtdir/images"
		if [ $srcfile -nt $tgtfilegz ];
		then
		    cd $base/$user/literatures/$literature/tpcas-1
		    # leave cd and second argument of xml2tpcas this way to get filename in tpcas file right
		    /usr/local/textpresso/bin/pdf2tpcas $srcfile $literature/$pdfdir
		    gzip $tgtfile
		    /usr/local/textpresso/bin/cmykinverter "$tgtdir/images"
		fi
	    done       
	done
	chown -R www-data:www-data $base/$user/literatures/$literature/tpcas-1/$literature
    done
done
rm -f /tmp/01_pdf2tpcas-1.lock
