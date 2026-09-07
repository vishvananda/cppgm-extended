#!/usr/bin/perl
use strict;
use warnings;
use FindBin;
use File::Basename qw(dirname);
my $repo_root = dirname(dirname($FindBin::Bin));
die "Usage: compare_results.pl <ref-suffix> <my-suffix> <test-spec>\n" if @ARGV != 3;
my ($reference, $candidate, $tests) = @ARGV;
exec("perl", "$repo_root/scripts/run_routed_test_spec.pl", $tests,
  "*tests/debuginfo/*::perl $repo_root/scripts/compare_results_common.pl text_t $reference $candidate {tests}",
  "*::perl $repo_root/scripts/compare_results_common.pl lowir_roundtrip_t $reference $candidate {tests}")
  or die "exec failed: $!";
