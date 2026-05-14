#!/usr/bin/perl
use strict;
use warnings;
use FindBin;

exec("perl", "$FindBin::Bin/run_all_tests_common.pl", "witness_t", @ARGV)
	or die "exec failed: $!";
