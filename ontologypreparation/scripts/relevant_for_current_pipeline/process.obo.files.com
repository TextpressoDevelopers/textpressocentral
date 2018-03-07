#!/bin/bash
#
# Before running this script, root-topparents relationships need to be defined
# in each obofile. topparents are discovered by the script find.root.terms.com
#
# $1 is directoryname that contains obofiles, for example /usr/local/textpresso/tpontology/obofiles
# $2 is the tree depth up to which the original structure engrained in the obofile will be preserved in tpontology
#
# #
# # still need to deal with removing old version in postgres
# #
#
# define elementary directories
#
bindir='/usr/local/textpresso/bin'
workrootdir='/usr/local/textpresso/tpontology/workdir'
#
$bindir/tpontology  create_table_tpontology
$bindir/tpontology create_table_pcrelations
$bindir/tpontology create_table_padcrelations
for i in $1/*.obo
do
    if [ -f $i ]
       then
	   fn=${i#$1/}
	   mkdir $workrootdir/$fn
	   cp $i $workrootdir/$fn/.
	   $bindir/obofileanalyzer $workrootdir/$fn/$fn
	   $bindir/tpontology convert_obo_to_tppc $workrootdir/$fn >$workrootdir/$fn/pc
	   $bindir/tpontology grow_tree $workrootdir/$fn/pc $workrootdir/$fn/$fn > $workrootdir/$fn/$fn.out
	   $bindir/tpontology populate_all_from_tppc_files $workrootdir/$fn $fn $2 www.textpresso.org
	   rm -rf $workrootdir/$fn
    fi
done
$bindir/generatelexicalvariations tmptpontology
