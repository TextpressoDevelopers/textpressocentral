#!/bin/bash
#
# run with following command (to avoid collisions)                         
# flock -n /tmp/wrapper.celegans.supplementals.lock -c ./wrapper.celegans.supplementals.com
#
cd /home/textpresso/tcpipelines/permanent/scripts
./find.new.celegans.supplementals.pdfs.and.copy.com
./pdf.2.tpcas-1.celegans.supplementals.com
./tpcas1.2.tpcas2.4.1.literature.com "C. elegans Supplementals"
rm /home/textpresso/tcpipelines/permanent/luceneindex/subindex.config
./literature.indexing.com "C. elegans Supplementals"
ls /home/textpresso/tcpipelines/permanent/luceneindex/ > /tmp/subindex.config
mv /tmp/subindex.config /home/textpresso/tcpipelines/permanent/luceneindex/subindex.config
rm -f /tmp/wrapper.celegans.supplementals.lock
