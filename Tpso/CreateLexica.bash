#!/bin/bash
# 
# File:   CreateLexica.bash
# Author: mueller
#
# Created on Sep 27, 2019, 12:17:27 AM
#
echo "drop table ontologymembers" | psql www-data
tpso -j /usr/local/etc/tpsoinput.json
echo "grant all privileges on table ontologymembers to \"www-data\"" | psql www-data
LIST=`echo "select tablename from pg_tables" | psql www-data | grep tpontology`
jmax=$(nproc)
jct=0
for i in $LIST
do
    generatelexicalvariations $i &
    jct=$((jct+1))
    if [[ $(($jct % $jmax)) == 0 ]]
    then
	wait
    fi
done
wait
