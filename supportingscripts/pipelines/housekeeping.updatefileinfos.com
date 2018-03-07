#!/bin/bash
/usr/local/textpresso/bin/tpfilehousekeeping -u /media/pmc/mueller/ftp.ncbi.nlm.nih.gov/
#/usr/local/textpresso/bin/tpfilehousekeeping -u /usr/local/textpresso/tpcas/
/usr/local/textpresso/bin/tpfilehousekeeping -u /mnt/data3/textpresso/wb/
/usr/local/textpresso/bin/tpfilehousekeeping -u /usr/local/textpresso/luceneindex/
/usr/local/textpresso/bin/tpfilehousekeeping -u /usr/local/textpresso/tpontology/plainlists
