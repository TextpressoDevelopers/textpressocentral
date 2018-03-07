#!/bin/bash

bin/tpontology create_table_pcrelations
bin/tpontology create_table_tpontology

for id in GO:0008150 GO:0005575 GO:0003674
do
    for i in $(cat obofiles/go/go.obo.elem.$id);
    do 
	echo $id $i;
    done > /tmp/$id; bin/tpontology populate_pcrelations_from_file /tmp/$id
    j=$(grep $id obofiles/go/go.obo.ids-names | cut -f 2); 
    echo bin/tpontology populate_tpontology_from_obo obofiles/go/go.obo.single.$id $id obofiles/go/go.obo.ids-names www.geneontology.org | batch
    for i in $(cat obofiles/go/go.obo.elem.$id);
    do
	echo bin/tpontology populate_tpontology_from_obo obofiles/go/go.obo.all.term.$i $i obofiles/go/go.obo.ids-names www.geneontology.org;
    done | batch
    for i in $(cat obofiles/go/go.obo.elem.$id);
    do
	echo bin/tpontology populate_tpontology_from_obo obofiles/go/go.obo.single.$i $i obofiles/go/go.obo.ids-names www.geneontology.org;
    done | batch
done

for id in CHEBI:33232 CHEBI:24432 CHEBI:51086 CHEBI:33250 CHEBI:59999 CHEBI:24433 CHEBI:23367
do
    for i in $(cat obofiles/chebi/chebi.obo.elem.$id);
    do
	echo $id $i; 
    done > /tmp/$id; bin/tpontology populate_pcrelations_from_file /tmp/$id
    echo bin/tpontology populate_tpontology_from_obo obofiles/chebi/chebi.obo.single.$id $id obofiles/chebi/chebi.obo.ids-names www.ebi.ac.uk/chebi/ | batch
    for i in $(cat obofiles/chebi/chebi.obo.elem.$id);
    do
	echo bin/tpontology populate_tpontology_from_obo obofiles/chebi/chebi.obo.all.term.$i $i obofiles/chebi/chebi.obo.ids-names www.ebi.ac.uk/chebi/;
    done | batch
    for i in $(cat obofiles/chebi/chebi.obo.elem.$id);
    do 
	echo bin/tpontology populate_tpontology_from_obo obofiles/chebi/chebi.obo.single.$i $i obofiles/chebi/chebi.obo.ids-names www.ebi.ac.uk/chebi/;
    done | batch
done

for id in PATO:0001241 PATO:0001236 PATO:0000068
do
    for i in $(cat obofiles/quality/quality.obo.elem.$id);
    do 
	echo $id $i;
    done > /tmp/$id; bin/tpontology populate_pcrelations_from_file /tmp/$id
    echo bin/tpontology populate_tpontology_from_obo obofiles/quality/quality.obo.single.$id $id obofiles/quality/quality.obo.ids-names bioportal.bioontology.org/ontologies/PATO | batch
    for i in $(cat obofiles/quality/quality.obo.elem.$id);
    do
	echo bin/tpontology populate_tpontology_from_obo obofiles/quality/quality.obo.all.term.$i $i obofiles/quality/quality.obo.ids-names  bioportal.bioontology.org/ontologies/PATO;
    done | batch
    for i in $(cat obofiles/quality/quality.obo.elem.$id);
    do
	echo bin/tpontology populate_tpontology_from_obo obofiles/quality/quality.obo.single.$i $i obofiles/quality/quality.obo.ids-names bioportal.bioontology.org/ontologies/PATO;  
    done | batch
done

for id in SO:0000400 SO:0001260 SO:0000110 SO:0001060
do
    for i in $(cat obofiles/so/so.obo.elem.$id);
    do
	echo $id $i;
    done > /tmp/$id; bin/tpontology populate_pcrelations_from_file /tmp/$id
    echo bin/tpontology populate_tpontology_from_obo obofiles/so/so.obo.single.$id $id obofiles/so/so.obo.ids-names www.sequenceontology.org | batch
    for i in $(cat obofiles/so/so.obo.elem.$id);
    do j=$(grep $i obofiles/so/so.obo.ids-names | cut -f 2);
	echo bin/tpontology populate_tpontology_from_obo obofiles/so/so.obo.all.term.$i $i obofiles/so/so.obo.ids-names www.sequenceontology.org;
    done | batch
    for i in $(cat obofiles/so/so.obo.elem.$id);
    do j=$(grep $i obofiles/so/so.obo.ids-names | cut -f 2);
	echo bin/tpontology populate_tpontology_from_obo obofiles/so/so.obo.single.$i $i obofiles/so/so.obo.ids-names www.sequenceontology.org;
    done | batch
done

for id in PR:000000001 GO:0043234
do
    for i in $(cat obofiles/pro/pro.obo.elem.$id);
    do 
	echo $id $i; 
    done > /tmp/$id; bin/tpontology populate_pcrelations_from_file /tmp/$id
    echo bin/tpontology populate_tpontology_from_obo obofiles/pro/pro.obo.single.$id $id obofiles/pro/pro.obo.ids-names pir.georgetown.edu/pro/ | batch
    for i in $(cat obofiles/pro/pro.obo.elem.$id);
    do 
	echo bin/tpontology populate_tpontology_from_obo obofiles/pro/pro.obo.all.term.$i $i obofiles/pro/pro.obo.ids-names pir.georgetown.edu/pro/;
    done | batch
    for i in $(cat obofiles/pro/pro.obo.elem.$id);
    do
	echo bin/tpontology populate_tpontology_from_obo obofiles/pro/pro.obo.single.$i $i obofiles/pro/pro.obo.ids-names pir.georgetown.edu/pro/;
    done | batch
done

echo bin/generatelexicalvariations | batch
