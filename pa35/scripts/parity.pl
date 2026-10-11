#!/usr/bin/env perl
# parity.pl [--reference C] [--levels O0,O3] [--counter auto|callgrind|perf] [--out DIR]
#
# How this compiler compares with the reference compiler on this machine,
# as information beside the gates (the gates are floors; this is the
# distance to the course solution):
#   speed    compile time and peak memory of each on the frozen workload,
#            as ratios to the host compiler (speed.pl on each)
#   codegen  generated code at constant work: this repository's own source
#            built by the reference compiler and by dev/cppgm++, both
#            counted on the workload (ab.pl), with the text size of each
# The reference is C, else reference-binaries/cppgm++ (after
# `make reference-binaries`), else dev/cppgm++-ref. Prints one table with
# the reference's figure, yours and the ratio yours / reference, and writes
# DIR/parity.tsv (default pa35/obj/parity).
use strict;
use warnings;
use FindBin;
use lib $FindBin::Bin;
use Cwd qw(abs_path);
use File::Path qw(make_path);
use Getopt::Long;
use PA35;

my %o = (levels => 'O0,O3', counter => 'auto', out => "$PA35::PA35/obj/parity");
GetOptions(\%o, 'reference=s', 'levels=s', 'counter=s', 'out=s')
	or die "usage: parity.pl [--reference C] [--levels O0,O3] [--counter auto|callgrind|perf] [--out DIR]\n";
my $reference = $o{reference} // (-x "$PA35::ROOT/reference-binaries/cppgm++"
	? "$PA35::ROOT/reference-binaries/cppgm++" : "$PA35::ROOT/dev/cppgm++-ref");
$reference = abs_path($reference) // die "no reference compiler $reference (run `make reference-binaries`)\n";
my $ours = abs_path("$PA35::ROOT/dev/cppgm++") // die "no dev/cppgm++ (run `make build`)\n";
make_path($o{out});
my $out = abs_path($o{out});
my $S = $FindBin::Bin;

my %row;    # measurement => [reference, ours]
sub speed_of {
	my ($label, $compiler) = @_;
	print "speed: $label ...\n";
	PA35::run_or_die($^X, "$S/speed.pl", '--compiler', $compiler, '--out', "$out/speed-$label.tsv");
	my %ratio;
	for my $line (split /\n/, PA35::slurp("$out/speed-$label.tsv")) {
		my ($metric, $c, $h, $r) = split /\t/, $line;
		$ratio{$metric} = $r if $metric ne 'metric';
	}
	return \%ratio;
}
my ($ref_speed, $our_speed) = (speed_of('reference', $reference), speed_of('yours', $ours));
for my $metric (qw(time-O0 time-O3 rss-O3)) {
	$row{"$metric (to host)"} = [$ref_speed->{$metric}, $our_speed->{$metric}];
}

# Constant work: both compilers build this source, and the two builds run
# the workload.
print "codegen: this source built by the reference and by dev/cppgm++ ...\n";
my ($st, $log) = PA35::run_capture($^X, "$S/ab.pl", $reference, $ours,
	'--levels', $o{levels}, '--counter', $o{counter}, '--out', "$out/ab");
die "ab.pl failed:\n$log" if $st;
for my $line (split /\n/, PA35::slurp("$out/ab/measure/summary.tsv")) {
	my ($level, $metric, $a, $b, $ratio) = split /\t/, $line;
	next if $level eq 'level' || $metric eq 'identical' || $metric eq 'calls';
	$row{"$level $metric instructions"} = [$a, $b];
}
my ($ta, $tb) = PA35::slurp("$out/ab/measure/summary.txt") =~ /text bytes: \S+ (\d+), \S+ (\d+)/;
$row{'text bytes'} = [$ta, $tb] if $ta;

my @lines;
printf "\n%-28s %14s %14s %8s\n", 'measurement', 'reference', 'yours', 'ratio';
for my $k (sort keys %row) {
	my ($a, $b) = @{ $row{$k} };
	next if !defined $a || !defined $b || $a !~ /^[\d.]+$/ || $b !~ /^[\d.]+$/;
	my $fmt = sub { $_[0] > 1e6 ? PA35::fmt_m($_[0]) : $_[0] =~ /\./ ? sprintf('%.4f', $_[0]) : $_[0] };
	my $ratio = $a > 0 ? sprintf('%.3f', $b / $a) : '-';
	printf "%-28s %14s %14s %8s\n", $k, $fmt->($a), $fmt->($b), $ratio;
	push @lines, join("\t", $k, $a, $b, $ratio);
}
PA35::spit("$out/parity.tsv", join("\n", "measurement\treference\tyours\tratio", @lines) . "\n");
print "\nratios above 1 are where the reference is ahead; wrote $out/parity.tsv\n";
