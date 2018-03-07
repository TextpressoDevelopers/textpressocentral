#!/bin/bash
#
# use absolute paths for $1 and $2
IFS=$(echo -en "\n\b")
for i in $1/*/images
do
    remainder=${i#"$1"}
    ln -s -T $i $2$remainder
done
