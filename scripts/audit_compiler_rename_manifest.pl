#!/usr/bin/perl

use strict;
use warnings;

use Cwd qw(abs_path);
use FindBin;

my $root = abs_path("$FindBin::Bin/..");
my $manifest = "$root/doc/compiler-rename-path-manifest.tsv";
my $baseline_commit = '5de0a619a05a176cc4bfc56393ff69c3260e6cf3';

# Only these removed course clients may retain a historical, absent owner.
# Keep the old rows: retirement must not erase their source provenance.
my %retirement_owner = (
	'dev/src/pa6_grammar_data.h' => 'dev/src/recognition/grammar_definition.h',
	'dev/src/pa6_recognizer.cpp' => 'dev/src/recognition/recognizer.cpp',
	'dev/src/pa6_recognizer.h' => 'dev/src/recognition/recognizer.h',
	'dev/src/pa7_semantic.cpp' => 'dev/src/namespace_semantics/analysis.cpp',
	'dev/src/pa7_semantic.h' => 'dev/src/namespace_semantics/analysis.h',
	'dev/src/pa8_internal.h' => 'dev/src/namespace_initialization/internal.h',
	'dev/src/pa8_model.cpp' => 'dev/src/namespace_initialization/model.cpp',
	'dev/src/pa8_parser.cpp' => 'dev/src/namespace_initialization/parser.cpp',
	'dev/src/pa8_program.h' => 'dev/src/namespace_initialization/program.h',
	'dev/src/pa8_semantic.cpp' => 'dev/src/namespace_initialization/driver.cpp',
	'dev/src/pa8_semantic.h' => 'dev/src/namespace_initialization/driver.h',
	'dev/src/pa8_types.h' => 'dev/src/namespace_initialization/types.h',
);

# The baseline is the last flat-layout commit of the source history.  A
# checkout that does not carry that history (the v4 tree grafted the
# implementation in) still checks the manifest's shape and every current
# owner; only the "is a baseline source" check needs the commit.
my %baseline;
my $have_baseline = system('git', '-C', $root, 'cat-file', '-e',
	"$baseline_commit^{tree}") == 0;
if ($have_baseline)
{
	open(my $git, '-|', 'git', '-C', $root, 'ls-tree', '-r', '--name-only',
		$baseline_commit, '--', 'dev/src')
		or die "unable to inspect baseline commit $baseline_commit: $!\n";
	while (my $path = <$git>)
	{
		chomp($path);
		$baseline{$path} = 1
			if $path =~ m{\Adev/src/pa\d[^/]*\.(?:cpp|h)\z};
	}
	close($git);
	die "unable to inspect baseline commit $baseline_commit\n" if $? != 0;
}

open(my $fh, '<', $manifest) or die "unable to read $manifest: $!\n";
my $header = <$fh>;
chomp($header) if defined($header);
die "$manifest has an invalid header\n" if !defined($header) ||
	$header ne "old_path\tcurrent_owner\tdisposition\treason";

my @error;
my %recorded;
my %count = map { $_ => 0 } qw(renamed split merged retired);
my $line = 1;
while (my $row = <$fh>)
{
	++$line;
	chomp($row);
	next if $row eq '';
	my ($old, $owners, $disposition, $reason, @extra) = split /\t/, $row, -1;
	if (@extra || !defined($reason))
	{
		push @error, "$manifest:$line must contain four fields";
		next;
	}
	push @error, "$manifest:$line duplicates '$old'" if $recorded{$old}++;
	push @error, "$manifest:$line '$old' is not a baseline PA source"
		if $have_baseline && !$baseline{$old};
	push @error, "$manifest:$line has invalid disposition '$disposition'"
		if $disposition !~ /\A(?:renamed|split|merged|retired)\z/;
	++$count{$disposition} if exists($count{$disposition});
	push @error, "$manifest:$line old path still exists: $old"
		if -e "$root/$old";
	if ($disposition eq 'retired')
	{
		push @error, "$manifest:$line retirement is not approved for '$old'"
			if !exists($retirement_owner{$old}) ||
				$owners ne $retirement_owner{$old};
		push @error, "$manifest:$line retirement needs a reason"
			if $reason !~ /\S/;
		push @error, "$manifest:$line retired owner still exists: $owners"
			if -e "$root/$owners";
		next;
	}
	push @error, "$manifest:$line live owner row has a retirement reason"
		if $reason ne '';

	my @owner = split /;/, $owners, -1;
	push @error, "$manifest:$line split row needs multiple owners"
		if $disposition eq 'split' && @owner < 2;
	push @error, "$manifest:$line non-split row needs exactly one owner"
		if $disposition ne 'split' && @owner != 1;
	my %owner;
	for my $owner (@owner)
	{
		push @error, "$manifest:$line duplicates current owner '$owner'"
			if $owner{$owner}++;
		push @error, "$manifest:$line has invalid current owner '$owner'"
			if $owner !~ m{\Adev/src/[^;]+\.(?:cpp|h)\z} ||
				$owner =~ m{(?:\A|/)pa\d}i;
		push @error, "$manifest:$line current owner is missing: $owner"
			if !-f "$root/$owner";
	}
}
close($fh) or die "unable to close $manifest: $!\n";

for my $old (sort keys %baseline)
{
	push @error, "$manifest has no row for baseline path '$old'"
		if !$recorded{$old};
}
for my $old (sort keys %recorded)
{
	push @error, "$manifest has a stale row for '$old'"
		if $have_baseline && !$baseline{$old};
}

if (@error)
{
	print STDERR "Compiler rename manifest audit failed with " .
		scalar(@error) . " error(s):\n";
	print STDERR "  $_\n" for @error;
	exit 1;
}

print "Compiler rename manifest audit passed: " . scalar(keys %baseline) .
	" baseline PA paths; $count{renamed} renamed, $count{split} split, and " .
	"$count{merged} merged into current responsibility owners; " .
	"$count{retired} retired with preserved provenance and reasons.\n";
