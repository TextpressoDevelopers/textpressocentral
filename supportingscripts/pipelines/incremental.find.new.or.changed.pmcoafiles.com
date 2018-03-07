#!/bin/bash
#
# need to run this as ./incremental.find.new.or.changed.pmcoafiles.com newlist oldlist
# use tpfilehousekeeping instead of this script
#
sdiff -w400 --suppress-common-lines $1 $2 > /tmp/d
NEW=$(grep '<' /tmp/d | cut -f 3 -d " ")
echo $NEW > new.articles
CHANGED=$(grep '|' /tmp/d | cut -f 3 -d " ")
echo $CHANGED > changed.articles
REMOVED=$(grep '>' /tmp/d | cut -f 9 -d " ")
echo $REMOVED > removed.articles
