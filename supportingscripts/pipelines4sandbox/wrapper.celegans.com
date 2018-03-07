#!/bin/bash
#
# run with following command (to avoid collisions)                         
# flock -n /tmp/wrapper.celegans.lock -c ./wrapper.celegans.com
#
cd /home/textpresso/tcpipelines/permanent/scripts
./find.new.celegans.pdfs.and.copy.com
./pdf.2.tpcas-1.celegans.com
./tpcas1.2.tpcas2.4.1.literature.com "C. elegans"
rm /home/textpresso/tcpipelines/permanent/luceneindex/subindex.config
./literature.indexing.com "C. elegans"
ls /home/textpresso/tcpipelines/permanent/luceneindex/ > /tmp/subindex.config
mv /tmp/subindex.config /home/textpresso/tcpipelines/permanent/luceneindex/subindex.config
rm -f /tmp/wrapper.celegans.lock
