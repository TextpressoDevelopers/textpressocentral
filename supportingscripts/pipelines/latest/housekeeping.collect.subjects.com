#!/bin/bash
#
rm -rf /usr/local/textpresso/etc/collection/*
mkdir -p /usr/local/textpresso/etc/collection/multiple
mkdir -p /usr/local/textpresso/etc/collection/unique
#
# Perlexpression: [Bb]io
# Subject: biology
#
grep -P '[Bb]io' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Biology'
#
# Perlexpression: [Nn]euro
# Subject: neuroscience
#
grep -P '[Nn]euro' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Neuroscience'
#
# Perlexpression: [Cc]anc|[Oo]nc
# Subject: oncology
#
grep -P '[Cc]anc|[Oo]nc' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Oncology'
#
# Perlexpression: [Mm]ethod
# Subject: methodology
#
grep -P '[Mm]ethod' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Methodology'
#
# Perlexpression: [Mm]ed
# Subject: med
#
grep -P '[Mm]edic' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Medicine'
#
# Perlexpression: [Vv]ir(us|ol)
# Subject: virology
#
grep -P '[Vv]ir(us|ol)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Virology'
#
# Perlexpression: [Gg]enet
# Subject: genetics
#
grep -P '[Gg]enet' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Genetics'
#
# Perlexpression: [Aa]nimal
# Subject: animal
#
grep -P '[Aa]nimal' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Animal'
#
# Perlexpression: [Cc]linic
# Subject: clinical
#
grep -P '[Cc]linic' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Clinical'
#
# Perlexpression: [Gg]enom
# Subject: genomics
#
grep -P '[Gg]enom' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Genomics'
#
# Perlexpression: [Dd]i(seas|abet)
# Subject: disease
#
grep -P '[Dd]i(seas|abet)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Disease'
#
# Perlexpression: [Aa]gricult
# Subject: agriculture
#
grep -P '[Aa]gricult' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Agriculture'
#
# Perlexpression: [Pp]hysiol
# Subject: physiology
#
grep -P '[Pp]hysiol' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Physiology'
#
# Perlexpression: [Pp]sych(ol|iat)
# Subject: psychology
#
grep -P '[Pp]sych(ol|iat)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Psychology'
#
# Perlexpression: [Cc]rystal
# Subject: crystallography
#
grep -P '[Cc]rystal' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Crystallography'
#
# Perlexpression: [Cc]hemi
# Subject: chemistry
#
grep -P '[Cc]hemi' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Chemistry'
#
# Perlexpression: [Hh]ealth
# Subject: health
#
grep -P '[Hh]ealth' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Health'
#
# Perlexpression: [Cc]ardi|[Hh]eart
# Subject: cardiology
#
grep -P '[Cc]ardi|[Hh]eart' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Cardiology'
#
# Perlexpression: [Pp]harm
# Subject: pharmacology
#
grep -P '[Pp]harm' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Pharmacology'
#
# Perlexpression: [Nn]utri
# Subject: nutrition
#
grep -P '[Nn]utri' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Nutrition'
#
# Perlexpression: [Ii]mmuno
# Subject: immunology
#
grep -P '[Ii]mmuno' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Immunology'
#
# Perlexpression: [Pp]a?ediatri
# Subject: pediatrics
#
grep -P '[Pp]a?ediatri' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Pediatrics'
#
# Perlexpression: [Rr]eview
# Subject: review
#
grep -P '[Rr]eview' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Review'
#
# Perlexpression: [Pp]rotein
# Subject: protein
#
grep -P '[Pp]rotein' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA Protein'
#
# Perlexpression: (Drosphila( melanogaster)?|[Ff]ruit [Ff]ly|D\. melanogaster)
# Subject: D. melanogaster
#
grep -P '(Drosphila( melanogaster)?|[Ff]ruit [Ff]ly|D\. melanogaster)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA D. melanogaster'
#
# Perlexpression: (Caenorhabditis( elegans)?|C\. elegans)
# Subject: C. elegans
#
grep -P '(Caenorhabditis( elegans)?|C\. elegans)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA C. elegans'
#
# Perlexpression: (Arabidopsis( thaliana)?|A\. thaliana)
# Subject: A. thaliana
#
grep -P '(Arabidopsis( thaliana)?|A\. thaliana)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA A. thaliana'
#
# Perlexpression: (Mus( musculus)?|M\. musculus|[Mm]usulus|[Mm]murine|[Mm]ouse|[Mm]ice)
# Subject: M. musculus
#
grep -P '(Mus( musculus)?|M\. musculus|[Mm]usulus|[Mm]murine|[Mm]ouse|[Mm]ice)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA M. musculus'
#
# Perlexpression: (Danio rerio|D\. rerio|[Zz]ebrafish)
# Subject: D. rerio
#
grep -P '(Danio rerio|D\. rerio|[Zz]ebrafish)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA D. rerio'
#
# Perlexpression: (Saccharomyces( cerevisiae)?|S\. cerevisiae|([Bb]aker\'s|[Bb]udding) [Yy]east)
# Subject: S. cerevisiae
#
grep -P '(Saccharomyces( cerevisiae)?|S\. cerevisiae|[Bb]udding [Yy]east)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA S. cerevisiae'
#
# Perlexpression: (Schizosaccharomyces( pombe)?|S\. pombe|([Ff]ission) [Yy]east)
# Subject: S. pombe
#
grep -P '(Schizosaccharomyces( pombe)?|S\. pombe|([Ff]ission) [Yy]east)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA S. pombe'
#
# Perlexpression: (Dictyostelium( discoideum)?|D\. discoideum|[Ss]lime [Mm]old)
# Subject: D. discoideum
#
grep -P '(Dictyostelium( discoideum)?|D\. discoideum|[Ss]lime [Mm]old)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA D. discoideum'
#
# Perlexpression: (Rattus norvegicus|R\. norvegicus|Norway brown rat)
# Subject: R. norvegicus
#
grep -P '(Rattus norvegicus|R\. norvegicus|Norway brown rat)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA R. norvegicus'
#
# Perlexpression: (Rattus rattus|R\. rattus|[Bb]lack rat) 
# Subject: R. rattus
#
grep -P '(Rattus rattus|R\. rattus|[Bb]lack rat)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA R. rattus'
#
# Perlexpression: (Ciona intestinalis|C\. intestinalis|[Ss]ea [Ss]quirt)
# Subject: C. intestinalies
#
grep -P '(Ciona intestinalis|C\. intestinalis|[Ss]ea [Ss]quirt)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA C. intestinalis'
#
# Perlexpression: (Xenopus laevis|X\. laevis|African clawed frog)
# Subject: X. laevis
#
grep -P '(Xenopus laevis|X\. laevis|African clawed frog)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA X. laevis'
#
# Perlexpression: (Xenopus tropicalis|X\. tropicalis|Western clawed frog)
# Subject: X. tropicalis
#
grep -P '(Xenopus tropicalis|X\. tropicalis|Western clawed frog)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA X. tropicalis'
#
# Perlexpression: (Escherichia coli|E\. coli)
# Subject: E. coli
#
grep -P '(Escherichia coli|E\. coli)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA E. coli'
#
# Perlexpression: (Bacillus subtilis|B\. subtilis)
# Subject B. subtilis
#
grep -P '(Bacillus subtilis|B\. subtilis)' /usr/local/textpresso/tmp/allnxml.subject.txt > '/usr/local/textpresso/etc/collection/multiple/PMCOA B. subtilis'
###
#
#
# Perlexpression: complement to all expressions above
# Subject: unclassified
#
#
cat /usr/local/textpresso/etc/collection/multiple/* > /tmp/allclassified
/usr/local/textpresso/sources/supportingscripts/pipelines/a-not-b.pl /usr/local/textpresso/tmp/allnxml.subject.txt /tmp/allclassified > '/usr/local/textpresso/etc/collection/multiple/PMCOA Unclassified'
#
###
#
#
# Classification above can assign one article to more than one subject.
# Make classification unique (i.e., assign an article to one subject only)
# by prioritizing according to list in for-loop below
#
#
for i in "PMCOA Biology" "PMCOA Neuroscience" "PMCOA Oncology" "PMCOA Methodology" "PMCOA Medicine" "PMCOA Virology" "PMCOA Genetics" "PMCOA Animal" "PMCOA Clinical" "PMCOA Genomics" "PMCOA Disease" "PMCOA Agriculture" "PMCOA Physiology" "PMCOA Psychology" "PMCOA Crystallography" "PMCOA Chemistry" "PMCOA Health" "PMCOA Cardiology" "PMCOA Pharmacology" "PMCOA Nutrition" "PMCOA Immunology" "PMCOA Pediatrics" "PMCOA Review" "PMCOA Protein" "PMCOA D. melanogaster" "PMCOA C. elegans" "PMCOA A. thaliana" "PMCOA M. musculus" "PMCOA D. rerio" "PMCOA S. cerevisiae" "PMCOA S. pombe" "PMCOA D. discoideum" "PMCOA R. norvegicus" "PMCOA R. rattus" "PMCOA C. intestinalis" "PMCOA X. laevis" "PMCOA X. tropicalis" "PMCOA E. coli" "PMCOA B. subtilis"
do 
    cat /usr/local/textpresso/etc/collection/unique/* >/tmp/blocked 2>/dev/null
    ~/ult/sources/supportingscripts/pipelines/a-not-b.pl "/usr/local/textpresso/etc/collection/multiple/$i" /tmp/blocked > "/usr/local/textpresso/etc/collection/unique/$i"
done
