#!/usr/bin/perl

use strict;

my $f1 = $ARGV[0];
my $f2 = $ARGV[1];

open (IN1, "<$f1");
my @l1 = <IN1>;
close (IN1);

open (IN2, "<$f2");
my @l2 = <IN2>;
close (IN2);

my %a1 = ();
foreach my $l (@l1) {
    chomp ($l);
    $a1{$l} = 1;
}

my %a2 = ();
foreach my $l (@l2) {
    chomp ($l);
    $a2{$l} = 1;
}

foreach my $k (keys % a1) {
    print $k, "\n" if (!$a2{$k});
}
