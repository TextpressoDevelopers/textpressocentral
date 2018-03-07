#!/bin/bash
#
# 
# run with following command (to avoid collisions)
# flock -n /tmp/06_makelitslive.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/06_makelitslive.com
#
export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/textpresso/lib/
IFS=$(echo -en "\n\b")
base="/usr/local/textpresso/useruploads"
cgiimagedir="/usr/lib/cgi-bin/tc/images"
tpcasdir="/usr/local/textpresso/tpcas"
lucenedir="/usr/local/textpresso/luceneindex"
## prepare all directories and files
for user in $(ls $base)
do
    rm $lucenedir/$user.subindex.config.new
    for literature in $(ls $base/$user/literatures)
    do
	rm -rf  $cgiimagedir/$literature.new
	mkdir -p $cgiimagedir/$literature.new
	ulbase="$base/$user/literatures/$literature"
	ln -s $ulbase/tpcas-2/$literature/* $cgiimagedir/$literature.new/.
	rm -rf $tpcasdir/$literature.new
	mkdir -p $tpcasdir/$literature.new
	cp -r $ulbase/tpcas-2/$literature/* $tpcasdir/$literature.new/.
	rm -rf $lucenedir/$literature.new
	mkdir -p $lucenedir/$literature.new
	cp -r $ulbase/luceneindextmp/* $lucenedir/$literature.new/.
	echo $literature >> $lucenedir/$user.subindex.config.new
    done
done
## make it live
for user in $(ls $base)
do
    for literature in $(ls $base/$user/literatures)
    do
	rm -rf $cgiimagedir/$literature.old
	mv $cgiimagedir/$literature $cgiimagedir/$literature.old
	mv $cgiimagedir/$literature.new $cgiimagedir/$literature
	chown -R www-data:www-data $cgiimagedir/$literature
	rm -rf $tpcasdir/$literature.old
	mv $tpcasdir/$literature $tpcasdir/$literature.old
	mv $tpcasdir/$literature.new $tpcasdir/$literature
	chown -R www-data:www-data $tpcasdir/$literature 
	rm -rf $lucenedir/$literature.old
	mv $lucenedir/$literature $lucenedir/$literature.old
	mv $lucenedir/$literature.new $lucenedir/$literature
	chown -R www-data:www-data $lucenedir/$literature
    done
    mv  $lucenedir/$user.subindex.config.new $lucenedir/$user.subindex.config
    chown www-data:www-data $lucenedir/$user.subindex.config
done
rm -f /tmp/06_makelitslive.lock
