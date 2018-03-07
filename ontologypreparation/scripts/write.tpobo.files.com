#!/bin/bash
#
# define elementary directories
#
bindir='/usr/local/textpresso/bin'
plainlistdir='/usr/local/textpresso/tpontology/plainlists'
obofiledir='/usr/local/textpresso/tpontology/tpobo'
#
#cat $plainlistdir/gene_celegans.txt | $bindir/tpflat2oboconverter "gene (C. elegans)" tpgce > $obofiledir/gene_celegans.obo
#cat $plainlistdir/molecule_uncategorized.txt | $bindir/tpflat2oboconverter "molecule (uncategorized)" tpmol > $obofiledir/molecule_uncategorized.obo
#cat $plainlistdir/protein_celegans.txt | $bindir/tpflat2oboconverter "protein (C. elegans)" tppce > $obofiledir/protein_celegans.obo
#cat $plainlistdir/variation_celegans.txt | $bindir/tpflat2oboconverter "variation (C. elegans)" tpvce > $obofiledir/variation_celegans.obo
#cat $plainlistdir/localization_verbs_082208.txt | $bindir/tpflat2oboconverter "ccc localization verbs" tpcccv > $obofiledir/localization_verbs_082208.obo
#cat $plainlistdir/localization_assay_terms_082208.txt | $bindir/tpflat2oboconverter "ccc localization assay" tpccca > $obofiledir/localization_assay_terms_082208.obo
#cat $plainlistdir/localization_cell_components_082208.txt | $bindir/tpflat2oboconverter "ccc localization components" tpcccc > $obofiledir/localization_cell_components_082208.obo
#cat $plainlistdir/mf_int_assay_2009-11-25.txt | $bindir/tpflat2oboconverter "mf interaction assay" tpmfia > $obofiledir/mf_int_assay_2009-11-25.obo
#cat $plainlistdir/mf_int_verbs_2009-11-25.txt | $bindir/tpflat2oboconverter "mf interaction verbs" tpmfiv > $obofiledir/mf_int_verbs_2009-11-25.obo
#cat $plainlistdir/mf_ea_assay-2011-02-18.txt | $bindir/tpflat2oboconverter "mf enz. activ. assay" tpmfeaa > $obofiledir/mf_ea_assay-2011-02-18.obo
#cat $plainlistdir/mf_ea_verbs-2011-02-18.txt | $bindir/tpflat2oboconverter "mf enz. activ. verbs" tpmfeav > $obofiledir/mf_ea_verbs-2011-02-18.obo
#cat $plainlistdir/disease_2015-01-29.txt | $bindir/tpflat2oboconverter "human disease" tpdi > $obofiledir/disease_2015-01-29.obo
#cat $plainlistdir/allele_celegans.2015-12-15_07_07_22.txt | $bindir/tpflat2oboconverter "allele (C. elegans)" tpalce > $obofiledir/allele_celegans.2015-12-15.obo
cat $plainlistdir/tables_and_figures_2016-06-15.txt | $bindir/tpflat2oboconverter "tables and figures" tptbfig > $obofiledir/tables_and_figures_2016-06-15.obo
cat $plainlistdir/cg_genetic_perturbation_2016-05-31.txt | $bindir/tpflat2oboconverter "genetic perturbation (WormBase curation)" tpgp > $obofiledir/cg_genetic_perturbation_2016-05-31.obo
cat $plainlistdir/cg_phenotypic_perturbation_2016-05-31.txt | $bindir/tpflat2oboconverter "phenotypic perturbation (WormBase curation)" tppp > $obofiledir/cg_phenotypic_perturbation_2016-05-31.obo
cat $plainlistdir/cg_genetic_interaction_2016-05-31.txt | $bindir/tpflat2oboconverter "genetic interaction (WormBase curation)" tpgi > $obofiledir/cg_genetic_interaction_2016-05-31.obo
