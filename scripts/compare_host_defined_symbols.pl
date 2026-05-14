#!/usr/bin/perl
use strict;
use warnings;

use File::Temp qw(tempdir);
use Text::ParseWords qw(shellwords);

sub canonical_symbol
{
	my ($symbol) = @_;
	if ($^O eq 'darwin' && defined($symbol) && $symbol =~ /^_/)
	{
		$symbol = substr($symbol, 1);
	}
	return $symbol;
}

sub run_capture
{
	my (@cmd) = @_;
	open(my $fh, '-|', @cmd) or die "exec failed (@cmd): $!";
	local $/;
	my $out = <$fh>;
	close($fh) or die "command failed (@cmd)";
	return defined($out) ? $out : '';
}

sub parse_defined_symbols
{
	my ($text) = @_;
	my @defined = ();
	for my $raw_line (split(/\n/, $text))
	{
		my $line = $raw_line;
		$line =~ s/^\s+//;
		$line =~ s/\s+$//;
		next if $line eq '';
		my @parts = split(/\s+/, $line);
		my ($kind, $symbol);
		if (@parts == 2)
		{
			($kind, $symbol) = @parts;
		}
		elsif (@parts >= 3)
		{
			($kind, $symbol) = @parts[1, 2];
		}
		else
		{
			next;
		}
		next if uc($kind) eq 'U';
		next if uc($kind) !~ /^(T|D|B|S|R|V|W)$/;
		push(@defined, $symbol);
	}
	return @defined;
}

sub demangle_symbol
{
	my ($symbol) = @_;
	my $out = run_capture('c++filt', $symbol);
	$out =~ s/\s+$//;
	return $out;
}

sub filtered_defined_symbols
{
	my ($obj, $filters_ref) = @_;
	my @defined = parse_defined_symbols(run_capture('nm', '-g', $obj));
	my %matched = ();
	if (!@$filters_ref)
	{
		$matched{canonical_symbol($_)} = 1 for @defined;
		return sort(keys(%matched));
	}

	SYMBOL:
	for my $symbol (@defined)
	{
		my $demangled = demangle_symbol($symbol);
		for my $fragment (@$filters_ref)
		{
			next SYMBOL if index($demangled, $fragment) < 0;
		}
		$matched{canonical_symbol($symbol)} = 1;
	}
	return sort(keys(%matched));
}

my $obj = '';
my $source = '';
my $host_cxx = '';
my $std = 'gnu++11';
my @compile_flags = ();
my @demangled_contains = ();

while (@ARGV)
{
	my $arg = shift(@ARGV);
	if ($arg eq '--obj')
	{
		$obj = shift(@ARGV) // '';
	}
	elsif ($arg eq '--source')
	{
		$source = shift(@ARGV) // '';
	}
	elsif ($arg eq '--host-cxx')
	{
		$host_cxx = shift(@ARGV) // '';
	}
	elsif ($arg eq '--std')
	{
		$std = shift(@ARGV) // '';
	}
	elsif ($arg eq '--compile-flag')
	{
		push(@compile_flags, shift(@ARGV) // '');
	}
	elsif ($arg eq '--demangled-contains')
	{
		push(@demangled_contains, shift(@ARGV) // '');
	}
	else
	{
		die "Unknown argument: $arg\n";
	}
}

die "Missing --obj\n" if $obj eq '';
die "Missing --source\n" if $source eq '';
die "Missing --host-cxx\n" if $host_cxx eq '';

my @host_cxx_argv = shellwords($host_cxx);
die "Empty --host-cxx\n" if !@host_cxx_argv;
if (!@compile_flags && defined($ENV{'CPPGM_STDLIB_FLAGS'}) && $ENV{'CPPGM_STDLIB_FLAGS'} ne '')
{
	push(@compile_flags, shellwords($ENV{'CPPGM_STDLIB_FLAGS'}));
}

my $tmpdir = tempdir('cppgm-host-define-XXXXXX', CLEANUP => 1);
my $host_obj = "$tmpdir/host.o";

my @compile_cmd = (
	@host_cxx_argv,
	@compile_flags,
	"-std=$std",
	'-x', 'c++',
	'-c',
	'-o', $host_obj,
	$source,
);

system(@compile_cmd) == 0 or die "host compile failed (@compile_cmd)\n";

my @host_symbols = filtered_defined_symbols($host_obj, \@demangled_contains);
my @obj_symbols = filtered_defined_symbols($obj, \@demangled_contains);

if (!@host_symbols)
{
	print STDERR "No matching host defined symbols found\n";
	exit 1;
}
if (!@obj_symbols)
{
	print STDERR "No matching implementation defined symbols found\n";
	exit 1;
}

my $host_join = join("\n", @host_symbols);
my $obj_join = join("\n", @obj_symbols);
if ($host_join ne $obj_join)
{
	print STDERR "Defined symbol mismatch against host compiler\n";
	print STDERR "host:\n";
	print STDERR "  $_\n" for @host_symbols;
	print STDERR "obj:\n";
	print STDERR "  $_\n" for @obj_symbols;
	exit 1;
}

print "host_defined_symbol_match 1\n";
exit 0;
