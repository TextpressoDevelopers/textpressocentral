#!/bin/bash
#
# 
# run with following command (to avoid collisions)
# flock -n /tmp/05_indexing.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/05_indexing.com
#
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
IFS=$(echo -en "\n\b")
base="/usr/local/textpresso/useruploads"
for user in $(ls $base)
do
    for literature in $(ls $base/$user/literatures)
    do
	ulbase="$base/$user/literatures/$literature/"
	indextimestamp="$ulbase/luceneindextmp/timestamp"
	unset flag;
	for source in $(find $ulbase/tpcas-2/$literature -name "*.gz")
	do
	    if [ $source -nt $indextimestamp ];
	    then
		flag=1
	    fi
	done
	for uploadedbib in $(find $ulbase/pdf -name "*.bib")
	    do
		dest="$ulbase/tpcas-2/$literature/${uploadedbib#$ulbase/pdf}"
		if [ -n "$(diff -q --new-file $uploadedbib $dest)" ]
		then
		flag=1
		fi
	    done
	if [ "$flag" ];
	then
	    cd $ulbase/tpcas-2/
	    # First deal with bib files.
	    # Do getbib4nxml. Writes bib for any nxml-based papers, otherwise produces an empty file
	    /usr/local/textpresso/bin/getbib4nxml $literature
	    # Copy bib template to all tpcas-2 directories that are pdf-based
	    for tpcas2dir in $(ls $ulbase/tpcas-2/$literature/)
	    do
		if [ -d $ulbase/pdf/$tpcas2dir ]
		then
		    cp /usr/local/textpresso/etc/template.bib $ulbase/tpcas-2/$literature/$tpcas2dir/$tpcas2dir.bib
		fi
	    done
	    # Last, copy bibs from pdf dirs, overwriting template when exists
	    for uploadedbib in $(find $ulbase/pdf -name "*.bib")
	    do
		dest="$ulbase/tpcas-2/$literature/${uploadedbib#$ulbase/pdf}"
	        cp $uploadedbib $dest
	    done
	    rm -r $ulbase/luceneindextmp/*
	    /usr/local/textpresso/bin/cas2singleindex $literature $ulbase/luceneindextmp/ new
	    touch $indextimestamp
	    chown -R www-data:www-data $ulbase/tpcas-2/
	fi
	chown -R www-data:www-data $ulbase/luceneindextmp/
    done
done
rm -f /tmp/05_indexing.lock
