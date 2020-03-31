#!/bin/bash
#
# Before running this script, root-topparents relationships need to be defined
# in each obofile.
#
# $1 is directoryname that contains obofiles, for example /usr/local/textpresso/tpontology/obofiles
#
# define elementary directories
#
bindir='/usr/local/textpresso/bin'
workrootdir='/usr/local/textpresso/tpontology/workdir'
#
echo "Each obofile should produce Tp:0000001 as the only root."
for i in $1/*.obo
do
#    echo "*** Processing $i..."
    fn=${i#$1/}
    rm -rf $workrootdir/$fn
    mkdir $workrootdir/$fn
    cp $i $workrootdir/$fn/.
    $bindir/obofileanalyzer $workrootdir/$fn/$fn
    $bindir/tpontology convert_obo_to_tppc $workrootdir/$fn >$workrootdir/$fn/pc
    for j in $(cut -f 1 $workrootdir/$fn/pc)
    do
	echo -n "$fn "
	echo -n "$j "; grep $j $workrootdir/$fn/pc | wc -l; 
    done | grep " 1$" | cut -f 1-2 -d " "
    rm -rf $workrootdir/$fn
#    echo "***"
done
