#!/bin/bash
# 
# File:   CreateLexica.bash
# Author: mueller
#
# Created on Sep 27, 2019, 12:17:27 AM
#
echo "drop table ontologymembers" | psql www-data
tpso -j $1/Tpso/input.json
echo "grant all privileges on table ontologymembers to \"www-data\"" | psql www-data
cd $1/GenerateLexicalVariations # need subdirectory resources
LIST=`echo "select tablename from pg_tables" | psql www-data | grep tpontology`
for i in $LIST
do
    generatelexicalvariations $i
done
wait
