#!/bin/bash
#
# run with following command (to avoid collisions)
# flock -n /tmp/00_remove_deleted.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/00_remove_deleted.com
#
#
base="/usr/local/textpresso/useruploads"
IFS=$(echo -en "\n\b")
for user in $(ls $base)
do
    for literature in $(ls $base/$user/literatures)
    do
	    for tpcas1 in $(ls $base/$user/literatures/$literature/tpcas-1/$literature)
	    do
		dir=$(find $base/$user/literatures/$literature -name "$tpcas1" | grep -v tpcas-1 | grep -v tpcas-2)
		if [ ! "$dir" ]
		then
		    rm -rf $base/$user/literatures/$literature/tpcas-1/$literature/$tpcas1
		fi
	    done
	    for tpcas2 in $(ls $base/$user/literatures/$literature/tpcas-2/$literature)
	    do
		if [ ! -d $base/$user/literatures/$literature/tpcas-1/$literature/$tpcas2 ]
		then
		    rm -rf $base/$user/literatures/$literature/tpcas-2/$literature/$tpcas2
		    rm -rf $base/$user/literatures/$literature/luceneindextmp/timestamp
		fi
	    done
    done
done
rm -f /tmp/00_remove_deleted.lock
