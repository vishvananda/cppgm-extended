#!/usr/bin/perl
# Check that every expectation sidecar is what regenerate_ir_expectations.pl
# writes from the current references, at its assignment's tolerance.
#
#   audit_ir_expectations.pl [--root DIR] [--backend TOOL] [--write]
#
# --backend names the reference lowir2native.  The behavior fixtures keep no
# machine IR reference, so their sidecars are checked only with it.  With
# --write the stale sidecars are rewritten.
use strict;
use warnings;
use File::Find;
use FindBin;
use Getopt::Long;

# The budget tolerance of each assignment's sidecars, in percent.
my %tolerance = (pa24 => 100, pa32 => 10, pa33 => 10);

my %o = (root => "$FindBin::Bin/..");
GetOptions(\%o, 'root=s', 'backend=s', 'write') && !@ARGV
	or die "usage: audit_ir_expectations.pl [--root DIR] [--backend TOOL] [--write]\n";

my %owners;
find({ no_chdir => 1, wanted => sub {
	return if !/\.ref\.expect$/ || !-f $_;
	my ($owner) = m{/(pa\d+)/tests/};
	die "audit_ir_expectations: no tolerance for $_\n"
		if !defined($owner) || !exists($tolerance{$owner});
	$owners{$owner} = 1;
} }, glob("$o{root}/pa*/tests"));

my @command = ($^X, "$FindBin::Bin/regenerate_ir_expectations.pl");
push @command, '--backend', $o{backend} if defined($o{backend});
push @command, '--write' if $o{write};
my $stale = 0;
for my $owner (sort keys %owners)
{
	system(@command, '--tolerance', $tolerance{$owner}, "$o{root}/$owner/tests") == 0
		or $stale = 1;
}
exit(0) if !$stale;
print STDERR "audit_ir_expectations: stale sidecars.  Regenerate the references, then run\n"
	. "  perl scripts/audit_ir_expectations.pl --backend dev/lowir2native-ref --write\n";
exit(1);
