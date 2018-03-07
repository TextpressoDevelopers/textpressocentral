#!/bin/bash
for i in `find $1 -name "*.tpcas"`
do
    gzip $i
done
