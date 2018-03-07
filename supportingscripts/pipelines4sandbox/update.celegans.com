#!/bin/bash
cd ~/ult/celegans_bib; rsync -a mueller@textpresso-dev.caltech.edu:/home/mueller/data2/data-processing/data/celegans/Data/includes/abstract/ abstract/
cd ~/ult/celegans_bib; rsync -a mueller@textpresso-dev.caltech.edu:/home/mueller/data2/data-processing/data/celegans/Data/includes/accession/ accession/
cd ~/ult/celegans_bib; rsync -a mueller@textpresso-dev.caltech.edu:/home/mueller/data2/data-processing/data/celegans/Data/includes/author/ author/
cd ~/ult/celegans_bib; rsync -a mueller@textpresso-dev.caltech.edu:/home/mueller/data2/data-processing/data/celegans/Data/includes/citation/ citation/
cd ~/ult/celegans_bib; rsync -a mueller@textpresso-dev.caltech.edu:/home/mueller/data2/data-processing/data/celegans/Data/includes/journal/ journal/
cd ~/ult/celegans_bib; rsync -a mueller@textpresso-dev.caltech.edu:/home/mueller/data2/data-processing/data/celegans/Data/includes/title/ title/
cd ~/ult/celegans_bib; rsync -a mueller@textpresso-dev.caltech.edu:/home/mueller/data2/data-processing/data/celegans/Data/includes/type/ type/
cd ~/ult/celegans_bib; rsync -a mueller@textpresso-dev.caltech.edu:/home/mueller/data2/data-processing/data/celegans/Data/includes/year/ year/
cd ~/tcpipelines/permanent/pdfs/rsync_from_textpresso_dev; rsync -a mueller@textpresso-dev.caltech.edu:/home/mueller/data2/data-processing/data/celegans/Data/processedfiles/pdf/*.sup.* C.\ elegans\ Supplementals/
cd ~/tcpipelines/permanent/pdfs/rsync_from_textpresso_dev; rsync -a --exclude=*.sup.* mueller@textpresso-dev.caltech.edu:/home/mueller/data2/data-processing/data/celegans/Data/processedfiles/pdf/ C.\ elegans/
flock -n /tmp/wrapper.celegans.lock -c /home/textpresso/tcpipelines/permanent/scripts/wrapper.celegans.com &
flock -n /tmp/wrapper.celegans.supplementals.lock -c /home/textpresso/tcpipelines/permanent/scripts/wrapper.celegans.supplementals.com &
wait
