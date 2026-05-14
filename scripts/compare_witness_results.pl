#!/usr/bin/perl
use strict;
use warnings;
use FindBin;

exec("perl", "$FindBin::Bin/compare_results_common.pl", "witness_t", @ARGV)
	or die "exec failed: $!";
