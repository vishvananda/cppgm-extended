#!/usr/bin/perl

use strict;
use warnings;

use File::Copy qw(copy);
use File::Temp qw(tempdir);
use FindBin;
use lib $FindBin::Bin;

use CppgmBatchWorker qw(collect_tests run_command_capture);

# At -O2 and above a loop head starts on an 8-byte boundary, on a 16-byte one
# when that takes at most 10 bytes of padding (dev/src/native/x86/emitter.cpp,
# align_loop_head). MIR does not show padding, so the check reads the host
# object's encoding: a loop head is the target of a backward branch. Levels
# below keep their heads where they fall, and the code stays contiguous.

sub read_file
{
	my ($path) = @_;
	open(my $fh, '<', $path) or die "Unable to read $path: $!\n";
	local $/;
	my $data = <$fh>;
	close($fh) or die "Unable to close $path: $!\n";
	return defined($data) ? $data : '';
}

sub object_function
{
	my ($test, $code, $symbol) = @_;
	return $1 if $code =~
		/(^[0-9a-f]+ <\@?\Q$symbol\E>:\n.*?)(?=^[0-9a-f]+ <|\z)/ms;
	die "$test: host object has no $symbol function\n";
}

# The offsets backward branches of one function's disassembly target.
sub loop_heads
{
	my ($code) = @_;
	my %heads;
	while($code =~ /^\s+([0-9a-f]+):\s+j\w+\s+([0-9a-f]+)\s/mg) {
		my ($from, $to) = (hex($1), hex($2));
		$heads{$to} = 1 if $to <= $from;
	}
	return sort { $a <=> $b } keys %heads;
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
	for my $level (qw(O1 O2 O3)) {
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
		my $disassembly = "$directory/$level.disassembly";
		$status = run_command_capture(
			cmd => ['objdump', '-d', '--no-show-raw-insn', $object],
			stdout => $disassembly,
			stderr => "$disassembly.stderr",
			timeout => 30,
		);
		die "$test: $level disassembly failed\n" if $status != 0;
		my $body = object_function($test, read_file($disassembly), 'nested');
		my @heads = loop_heads($body);
		die "$test: $level has no two loops to read\n" if scalar(@heads) < 2;
		next if $level eq 'O1';
		for my $head (@heads) {
			die sprintf("%s: %s loop head at %x is off its boundary\n",
				$test, $level, $head) if $head % 8 != 0;
		}
	}
}

print "host loop alignment: PASS (" . scalar(@tests) . "/" .
	scalar(@tests) . ")\n" unless $ENV{CPPGM_REPORT_QUIET};
