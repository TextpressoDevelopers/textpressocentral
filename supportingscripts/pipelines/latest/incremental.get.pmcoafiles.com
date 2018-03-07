#!/bin/bash
wget -r -nc -R *.pdf ftp://ftp.ncbi.nlm.nih.gov/pub/pmc/oa_package/
