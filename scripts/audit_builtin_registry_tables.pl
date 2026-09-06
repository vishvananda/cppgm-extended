#!/usr/bin/perl

# The hosted builtin registry answers every lookup with a binary search over a
# static table, so each table has to stay sorted by spelling.  An entry added in
# the wrong place does not fail to build and does not fail most tests: the
# builtin simply stops being recognised, and the failure surfaces far away as
# an unknown name in whichever header happened to use it.

use strict;
use warnings;

use Cwd qw(abs_path);
use FindBin;

my $root = abs_path("$FindBin::Bin/..");
my $path = "$root/dev/src/preprocess/hosted/builtin_registry.cpp";

open(my $fh, '<', $path) or die "unable to read $path: $!\n";
my @lines = <$fh>;
close($fh);

my @errors;
my $tables = 0;
my $entries = 0;
my $table_name;
my $table_line;
my $previous;
for (my $i = 0; $i < @lines; ++$i)
{
	my $line = $lines[$i];
	# Any file-scope table whose entries begin with a spelling is searched the
	# same way, whether it is an Entry<> or a bespoke struct.
	if ($line =~ /^(?:static\s+)?const\s+\S+(?:<\w+>)?\s+(\w+)\[\]\s*=\s*\{/)
	{
		$table_name = $1;
		$table_line = $i + 1;
		$previous = undef;
		++$tables;
		next;
	}
	next if !defined($table_name);
	if ($line =~ /^\s*\};/)
	{
		$table_name = undef;
		next;
	}
	next if $line !~ /\{\s*"([^"]+)"/;
	my $spelling = $1;
	++$entries;
	if (defined($previous) && $spelling lt $previous)
	{
		push @errors, "$table_name (declared at line $table_line): " .
			"\"$spelling\" at line " . ($i + 1) . " sorts before \"$previous\"";
	}
	$previous = $spelling;
}

if (@errors)
{
	print STDERR "builtin registry table audit: FAIL\n";
	print STDERR "  $_\n" for @errors;
	exit 1;
}

die "builtin registry table audit found no tables in $path\n" if !$tables;

# GetFloatingIntrinsic indexes the table by the enumerator's own position, so
# the enum has to list the kinds in the table's order.  A mismatch does not
# fail to build: every intrinsic silently answers as some other one, which
# surfaced here as a segmentation fault and as an unrelated test regression.
my $header = "$root/dev/src/preprocess/hosted/builtin_registry.h";
open(my $hh, '<', $header) or die "unable to read $header: $!\n";
my @header_lines = <$hh>;
close($hh);

my @table_kinds;
for my $line (@lines)
{
	push @table_kinds, $1 if $line =~ /\{"[^"]+",\s*(FLOATING_INTRINSIC_\w+)\s*,/;
}

my @enum_kinds;
my $in_enum = 0;
for my $line (@header_lines)
{
	$in_enum = 1 if $line =~ /enum FloatingIntrinsicKind\b/;
	next if !$in_enum;
	last if $line =~ /FLOATING_INTRINSIC_COUNT/;
	next if $line =~ /FLOATING_INTRINSIC_NONE/;
	push @enum_kinds, $1 if $line =~ /^\s*(FLOATING_INTRINSIC_\w+),/;
}

if (scalar(@table_kinds) != scalar(@enum_kinds))
{
	print STDERR "builtin registry table audit: FAIL\n";
	print STDERR "  floating intrinsic table has " . scalar(@table_kinds) .
		" kinds but the enum lists " . scalar(@enum_kinds) . "\n";
	exit 1;
}
for (my $i = 0; $i < @table_kinds; ++$i)
{
	next if $table_kinds[$i] eq $enum_kinds[$i];
	print STDERR "builtin registry table audit: FAIL\n";
	print STDERR "  floating intrinsic $i is $table_kinds[$i] in the table " .
		"but $enum_kinds[$i] in the enum\n";
	exit 1;
}

print "builtin registry table audit: PASS ($tables tables, $entries entries, " .
	scalar(@table_kinds) . " floating kinds in order)\n";
exit 0;
