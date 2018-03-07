#!/bin/bash
#
flock -n /tmp/00_remove_deleted.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/00_remove_deleted.com 1>/dev/null 2>&1
flock -n /tmp/01_pdf2tpcas-1.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/01_pdf2tpcas-1.com 1>/dev/null 2>&1
flock -n /tmp/02_nxml2tpcas-1.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/02_nxml2tpcas-1.com 1>/dev/null 2>&1
flock -n /tmp/03_nxmltar2tpcas-1.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/03_nxmltar2tpcas-1.com 1>/dev/null 2>&1
flock -n /tmp/04_tpcas1.to.tpcas2.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/04_tpcas1.to.tpcas2.com 1>/dev/null 2>&1
flock -n /tmp/05_indexing.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/05_indexing.com 1>/dev/null 2>&1
flock -n /tmp/06_makelitslive.lock -c /usr/local/textpresso/sources/supportingscripts/useruploadpipelines/06_makelitslive.com 1>/dev/null 2>&1
