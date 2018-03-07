#!/bin/bash
wget -r --wait=1 -R *.pdf ftp://ftp.ncbi.nlm.nih.gov/pub/pmc/
