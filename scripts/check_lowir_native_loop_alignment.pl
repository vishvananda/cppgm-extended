#!/usr/bin/perl

use strict;
use warnings;

use File::Copy qw(copy);
use File::Temp qw(tempdir);
use FindBin;
use lib $FindBin::Bin;

use CppgmBatchWorker qw(collect_tests run_command_capture);

# Check behavior and a broad footprint envelope for nested loops in a host
# object. A particular loop alignment is not a quality contract: padding an
# initialization loop can shift later hot code and lose runtime performance.
# Native timing, rather than a required boundary or branch encoding, decides
# whether a layout policy is profitable.

sub read_file
{
	my ($path) = @_;
	open(my $fh, '<', $path) or die "Unable to read $path: $!\n";
	local $/;
	my $data = <$fh>;
	close($fh) or die "Unable to close $path: $!\n";
	return defined($data) ? $data : '';
}

if(scalar(@ARGV) != 2)
{
	die "Usage: check_lowir_native_loop_alignment.pl " .
		"<cppgm++> <test-or-directory>\n";
}

my ($driver, $root) = @ARGV;
my @tests = collect_tests($root, qr/host-loop-alignment\.t$/);
die "No host loop-alignment controls found under $root\n" if !@tests;

for my $test (@tests)
{
	my $directory = tempdir('native-loop-alignment-XXXXXX',
		TMPDIR => 1, CLEANUP => 1);
	my $input = "$directory/input.lowir";
	copy($test, $input) or die "$test: unable to prepare the input: $!\n";
	for my $level (qw(O0 O1 O2 O3)) {
		my $object = "$directory/$level.o";
		my $status = run_command_capture(
			cmd => [$driver, '-c', "-$level", '-o', $object, $input],
			stdout => "$directory/$level.compile.stdout",
			stderr => "$directory/$level.compile.stderr",
			timeout => 60,
		);
		die "$test: $level object compile failed\n" .
			read_file("$directory/$level.compile.stderr") if $status != 0;
		my $program = "$directory/$level.program";
		$status = run_command_capture(
			cmd => [$driver, '-o', $program, $object],
			stdout => "$directory/$level.link.stdout",
			stderr => "$directory/$level.link.stderr",
			timeout => 60,
		);
		die "$test: $level link failed\n" if $status != 0;
		$status = run_command_capture(
			cmd => [$program],
			stdout => "$directory/$level.run.stdout",
			stderr => "$directory/$level.run.stderr",
			timeout => 30,
		);
		die "$test: $level behavior failed with status $status\n"
			if $status != 0;
		next if $level eq 'O0' || $level eq 'O1';
		my $symbols = "$directory/$level.symbols";
		$status = run_command_capture(
			cmd => ['objdump', '-t', $object],
			stdout => $symbols,
			stderr => "$symbols.stderr",
			timeout => 30,
		);
		die "$test: $level symbol table failed\n" if $status != 0;
		my $bytes;
		for my $line (split /\n/, read_file($symbols)) {
			my @fields = split ' ', $line;
			next unless @fields >= 5 && $fields[-1] =~ /^\@?nested$/;
			next unless $fields[-2] =~ /^[0-9a-f]+$/i && $fields[-3] =~ /^\.text(?:\.|$)/;
			$bytes = hex($fields[-2]);
		}
		die "$test: $level host object has no nested function\n" unless defined $bytes && $bytes > 0;
		die "$test: $level nested function uses $bytes bytes (limit 512)\n" if $bytes > 512;
	}
}

print "host loop layout: PASS (" . scalar(@tests) . "/" .
	scalar(@tests) . ")\n" unless $ENV{CPPGM_REPORT_QUIET};
