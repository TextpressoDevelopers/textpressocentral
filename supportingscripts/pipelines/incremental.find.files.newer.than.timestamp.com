#!/bin/bash
# 
# $1 is the directory to work on
# $2 is the timestamp to be compared against (see touch command)
# $3 is the filter filename pattern (see find command)
####
touch -t $2 /tmp/$2 
find -L $1/ -newer /tmp/$2 -name "$3"
