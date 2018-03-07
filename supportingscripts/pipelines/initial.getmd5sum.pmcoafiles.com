#!/bin/bash
LIST=$(find ftp.ncbi.nlm.nih.gov/pub/pmc/*/ -name "*.gz")
random=$(date +%Y%m%d%H%M%S);
sleep 1
rm -rf md5sum.$random.list
for i in $LIST
do
md5sum $i >> md5sum.$random.list
done
