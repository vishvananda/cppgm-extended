#!/usr/bin/perl

# The scaffold files the student export ships from dev/src (dev_support_files
# in scripts/export_student_repo.sh) are the students' starting points.  The
# implementation uses some of them as shipped and replaces others with
# headers of its own, which declare different entities under the same names:
# a translation unit cannot include both, and a program built from both would
# break the one-definition rule without a diagnostic.
#
# Fails when a shipped file is missing, or when an implementation source
# includes a shipped header the implementation replaces.  The student
# scaffolds (dev/*-scaffold.cpp) and the shipped files themselves may include
# them.

use strict;
use warnings;

use Cwd qw(abs_path);
use File::Find qw(find);
use FindBin;

my $root = abs_path("$FindBin::Bin/..");

# The shipped headers the implementation replaces, and its own headers.
my %replaced = (
	'abi/itanium/abi_mangle.h' => 'abi/itanium/abi_encoder.h',
	'abi/itanium/abi_mangle_expression.h' => 'abi/itanium/abi_expression_vocabulary.h',
	'abi/itanium/abi_mangle_facts.h' => 'abi/itanium/abi_graph.h',
	'abi/itanium/abi_mangle_reference.h' => 'abi/itanium/abi_graph.h',
	'abi/itanium/abi_mangle_stats.h' => 'abi/itanium/abi_stats.h',
	'abi/itanium/abi_mangle_terminal.h' => 'abi/itanium/abi_terminal_vocabulary.h',
	'abi/itanium/abi_mangle_type_vocabulary.h' => 'abi/itanium/abi_type_vocabulary.h',
	'preprocess/tokens/DebugPPTokenStream.h' => 'preprocess/tokens/debug_pp_token_stream.h',
	'support/tool_help_text.h' => 'support/tool_usage.h',
);

sub read_text
{
	my ($path) = @_;
	open(my $fh, '<', $path) or die "unable to read $path: $!\n";
	local $/;
	my $text = <$fh>;
	close($fh) or die "unable to close $path: $!\n";
	return defined($text) ? $text : '';
}

my @errors;
my %shipped;
if (read_text("$root/scripts/export_student_repo.sh") =~ /^dev_support_files=\(\n(.*?)^\)/ms)
{
	my $list = $1;
	$shipped{$_} = 1 for $list =~ /^\s*(dev\/\S+)\s*$/mg;
}
push @errors, "scripts/export_student_repo.sh lists no dev_support_files" if !%shipped;
for my $path (sort keys %shipped)
{
	push @errors, "shipped scaffold file $path is missing" if !-f "$root/$path";
}
for my $header (sort keys %replaced)
{
	push @errors, "replaced header dev/src/$header is not a shipped scaffold file"
		if !$shipped{"dev/src/$header"};
	push @errors, "dev/src/$replaced{$header}, which replaces dev/src/$header, is missing"
		if !-f "$root/dev/src/$replaced{$header}";
}

my @sources;
find({
	wanted => sub {
		push @sources, $File::Find::name if -f $_ && /\.(?:cpp|h)\z/;
	},
	no_chdir => 1,
}, "$root/dev/src");
push @sources, grep { -f $_ && !/-scaffold\.cpp\z/ } glob("$root/dev/*.cpp");

my $checked = 0;
for my $path (sort @sources)
{
	(my $rel = $path) =~ s{\A\Q$root\E/}{};
	next if $shipped{$rel};
	++$checked;
	my $line = 0;
	for my $text (split /\n/, read_text($path))
	{
		++$line;
		next if $text !~ /^\s*#\s*include\s*"([^"]+)"/;
		my $header = $1;
		next if !$replaced{$header};
		push @errors, "$rel:$line includes the shipped scaffold header $header, " .
			"which the implementation replaces; include $replaced{$header}";
	}
}

if (@errors)
{
	print STDERR "Scaffold header audit: FAIL (" . scalar(@errors) . " issue(s))\n";
	print STDERR "  $_\n" for @errors;
	exit 1;
}

print "Scaffold header audit: PASS (" . scalar(keys %shipped) . " shipped files; " .
	"$checked implementation sources include none of the " . scalar(keys %replaced) .
	" headers the implementation replaces)\n";
