#!/bin/bash
IFS=$(echo -en "\n\b")
cut -f 2 /usr/local/textpresso/tmp/allnxml.subject.txt | cut -f 2 -d ":" | cut -f 1-2 -d "," | sort | uniq  > /usr/local/textpresso/tmp/uniq.subjects.txt
for i in $(cat /usr/local/textpresso/tmp/uniq.subjects.txt ); do n=$(grep "Subject:$i" /usr/local/textpresso/tmp/allnxml.subject.txt | wc -l) ; echo $n $i; done | sort -n -r
