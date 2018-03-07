#!/usr/bin/perl -w

use strict;
use diagnostics;
use DBI;
use POSIX qw/strftime/;
use lib "/usr/local/lib/textpresso/celegans";

my $dbh = DBI->connect ( "dbi:Pg:dbname=testdb;host=131.215.52.76", "acedb", "") or die "Cannot connect to database!\n"; 

print "Processing Karen's drug/molecule names...\n";
my %mop_publicname = ();
my $result = $dbh->prepare("SELECT * FROM mop_publicname WHERE mop_publicname IS NOT NULL");
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[1]) {
	$mop_publicname{$row[1]} = 1;
    } 
}

print "Processing Karen's drug/molecule synonyms...\n";
my %mop_synonyms = ();
$result = $dbh->prepare("SELECT * FROM mop_synonym WHERE mop_synonym IS NOT NULL");
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[1]) {
	my @aux = split / \| /, $row[1];
	foreach my $a (@aux) {
	    $mop_synonyms{$a} = 1;
	}
    } 
}

print "Processing gin_wbgene...\n";
my %gin_wbgene = ();
$result = $dbh->prepare( "SELECT * FROM gin_wbgene WHERE gin_wbgene IS NOT NULL" );
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[1]) { 
	$gin_wbgene{$row[1]} = 1 if (is_celegans_gene($row[1]));
    } 
}

print "Processing gin_synonyms...\n";
my %gin_synonyms = ();
$result = $dbh->prepare( "SELECT * FROM gin_synonyms WHERE gin_synonyms IS NOT NULL" );
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[1]) { 
	$gin_synonyms{$row[1]} = 1 if (is_celegans_gene($row[1]));
    } 
}

print "Processing gin_molname...\n";
my %gin_molname = ();
$result = $dbh->prepare( "SELECT * FROM gin_molname WHERE gin_molname IS NOT NULL" );
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[1]) { 
	$gin_molname{$row[1]} = 1 if (is_celegans_gene($row[1]));
    } 
}

print "Processing gin_seqprot...\n";
my %gin_seqprot = ();
$result = $dbh->prepare( "SELECT * FROM gin_seqprot WHERE gin_seqprot IS NOT NULL" );
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[1]) { 
	$gin_seqprot{$row[1]} = 1 if (is_celegans_gene($row[1]));
    } 
}

print "Processing genesequencelab...\n";
my %gin_genesequencelab = ();
$result = $dbh->prepare( "SELECT * FROM gin_genesequencelab WHERE gin_genesequencelab IS NOT NULL" );
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[0]) { 
	$gin_genesequencelab{$row[0]} = 1 if (is_celegans_gene($row[0]));
    } 
}

print "Processing locus...\n";
my %gin_locus = ();
$result = $dbh->prepare("SELECT * FROM gin_locus WHERE gin_locus IS NOT NULL");
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[1]) {
	$gin_locus{$row[1]} = 1 if (is_celegans_gene($row[1]));
    }
}

print "Processing seqname...\n";
my %gin_seqname = ();
$result = $dbh->prepare("SELECT * FROM gin_seqname WHERE gin_seqname IS NOT NULL");
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[1]) { 
	$gin_seqname{$row[1]} = 1 if (is_celegans_gene($row[1]));
    } 
}

print "Processing sequence...\n";
my %gin_sequence = ();
$result = $dbh->prepare("SELECT * FROM gin_sequence WHERE gin_sequence IS NOT NULL");
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[1]) { 
	$gin_sequence{$row[1]} = 1 if (is_celegans_gene($row[1]));
    } 
}

print "Processing gin_variation...\n";
my %gin_variation = ();
$result = $dbh->prepare("SELECT gin_variation FROM gin_variation WHERE gin_variation IS NOT NULL");
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[0]) { 
	$gin_variation{$row[0]} = 1;
    } 
}
print "Processing obo_name_variation...\n";
my %obo_name_variation = ();
$result = $dbh->prepare("SELECT obo_name_variation FROM obo_name_variation WHERE obo_name_variation IS NOT NULL");
$result->execute() or die "Cannot prepare statement: $DBI::errstr\n"; 
while (my @row = $result->fetchrow) {
    if ($row[0]) { 
	$obo_name_variation{$row[0]} = 1;
    } 
}

$dbh->disconnect;

#my $date = strftime('%F_%H_%M_%S',localtime);
my %all = ();
#####
# %mop_publicname = ();
# %mop_synonyms = ();
foreach my $k (keys % mop_publicname ,keys % mop_synonyms) {
    $all{$k} = 1;
}
print "Printing molecule list...\n";
print_l(\%all, "molecule_uncategorized.txt");

%all = ();
#####
# %gin_wbgene = ();
# %gin_synonyms = ();
# %gin_molname = ();
# %gin_seqprot = ();
# %gin_genesequencelab = ();
# %gin_locus = ();
# %gin_seqname = ();
# %gin_sequence = ();

my %protein_name = ();
foreach my $k (keys % gin_wbgene, keys % gin_synonyms, keys % gin_molname, 
	       keys % gin_seqprot, keys % gin_genesequencelab, keys % gin_locus, 
	       keys % gin_seqname, keys % gin_sequence) {
    $all{$k} = 1;
    $protein_name{"\U$k\E"} = 1 if ($k =~ /^[a-z]{3,4}-\d+/);
    $protein_name{$k} = 1 if ($k =~ /^CE\d+/);
}

print "Printing gene list...\n";
print_l(\%all, "gene_celegans.txt");

print "Printing protein list...\n";
print_l(\%protein_name, "protein_celegans.txt");

%all = ();
foreach my $k (keys % gin_variation, keys % obo_name_variation) {
    $all{$k} =1;
}

print "Printing allele list...\n";
print_l(\%all, "variation_celegans.txt");

sub print_l {

    my $plist = shift;
    my $fname = shift;

    open (OUT, ">$fname");
    foreach my $k (keys % $plist) {
	$k =~ s/\\//g; # remove all backslashes
	print OUT $k, "\n";
    }
    close (OUT);
}

sub is_celegans_gene {
    my $s = shift;
    if ($s =~ /^(WBGene|GENEPREDICTION|Cr|Cbr|Cbg|Cbn|Cjp|Hpa|Oti|Ppa|Cja)/i) {
	return 0;
    }
    return 1;
}
