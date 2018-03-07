#!/bin/bash
#
# run with following command (to avoid collisions)
# flock -n /tmp/pdf.2.tpcas-1.celegans.lock -c ./pdf.2.tpcas-1.celegans.com
#
#
base="/usr/local/textpresso/useruploads"
IFS=$(echo -en "\n\b")
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
incomingdir="/home/textpresso/tcpipelines/permanent/pdfs"
tpcas1dir="/home/textpresso/tcpipelines/permanent/tpcas-1"
literature="C. elegans"
cd "$tpcas1dir"
for pdfdir in $(ls "$incomingdir/$literature")
do
    tgtdir="$tpcas1dir/$literature/$pdfdir"
    for pdf in $(ls "$incomingdir/$literature/$pdfdir" | grep .pdf)
    do
	filename=$(basename "$pdf")
	filename="${filename%.*}"
	srcfile="$incomingdir/$literature/$pdfdir/$filename.pdf"
	tgtfile="$tgtdir/$filename.tpcas"
	tgtfilegz="$tgtdir/$filename.tpcas.gz"
        mkdir -p "$tgtdir"
        mkdir -p "$tgtdir/images"
        if [ $srcfile -nt $tgtfilegz ];
        then
            # leave cd and second argument of pdf2tpcas this way to get filename in tpcas file right
            timeout 1m /usr/local/textpresso/bin/pdf2tpcas $srcfile $literature/$pdfdir
            gzip $tgtfile
            timeout 1m /usr/local/textpresso/bin/cmykinverter "$tgtdir/images"
        fi
    done       
done
rm -f /tmp/pdf.2.tpcas-1.celegans.lock
