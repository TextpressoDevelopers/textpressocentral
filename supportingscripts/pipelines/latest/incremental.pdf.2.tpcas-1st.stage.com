#!/bin/bash
#
# runs as incremental.pdf.2.tpcas-1st.stage.com <list of subdir> <pdf dir>
#
# The <tpcas 1st stage dir> needs to be the current work directory  
#
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
IFS=$(echo -en "\n\b")
for i in $(cat "$1");
do
    mkdir -p "$i/images"
    for j in $(find -L "$2/$i/" -name "*.pdf")
    do
	/usr/local/textpresso/bin/pdf2tpcas "$j" "$i"
    done
    for j in $(find -L "$i" -name "*.tpcas")
    do
        gzip "$j"
    done
    /usr/local/textpresso/bin/cmykinverter "$i/images"
done
