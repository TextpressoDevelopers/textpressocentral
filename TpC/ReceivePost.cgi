#!/usr/bin/perl
# This is a small server that receives a HTTP POST
use strict;
use warnings FATAL => 'all';
use CGI qw();
my $cgi = new CGI;
my $xml = $cgi->param('POSTDATA');
print $cgi->header;
print $cgi->start_html(-title => "POST SERVER RESPONSE");
print $cgi->span($xml);
print $cgi->end_html;

