#!/bin/bash
rm -f a.list
rm -f b.list
for i in `cat $1`
do
 if [ "$RANDOM" -gt "$2" ]
 then
     echo $i >> a.list
 else
    echo $i >> b.list
 fi
done
