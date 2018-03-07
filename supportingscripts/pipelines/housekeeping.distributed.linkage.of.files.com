#!/bin/bash
IFS=$(echo -en "\n\b")
if [ -d "$1" ] ; then
    for i in `ls $1`
    do
	j=${i:0:2}
	mkdir -p $2/$j
	ln -s $1/$i $2/$j/
    done
fi
