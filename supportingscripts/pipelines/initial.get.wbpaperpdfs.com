#!/bin/bash
wget -r -np http://textpresso-dev.caltech.edu/celegans/tdb/celegans/txt/bib-all/
rsync -a textpresso-dev.caltech.edu://data2/srv/textpresso-dev.caltech.edu/www/docroot/celegans/tdb/celegans/txt/pdf .
