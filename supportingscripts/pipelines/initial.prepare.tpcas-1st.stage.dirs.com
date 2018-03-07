#!/bin/bash
#
# $1 full path to nxmls
# $2 path of destination dir (tpcas-1st.stage)
for i in $(ls $1)
do
    mkdir -p $2/$i
    ln -s $1/$i/images $2/$i/.
done
