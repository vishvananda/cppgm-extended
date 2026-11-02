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
# object's encoding: a loop head is the target of a backward branch. Only an
# innermost head must be on a boundary, one with no other head before its
# branch: a block the block before it mostly falls into takes no padding.
# Levels below keep their heads where they fall, and the code stays contiguous.
# An innermost loop of at most 32 bytes stays in one 32-byte window of a text
# section aligned to 32.

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
		next if $to > $from;
		$heads{$to} = $from if !defined($heads{$to}) || $heads{$to} < $from;
	}
	return %heads;
}

# The offset of the instruction after the one at `from`.
sub instruction_end
{
	my ($code, $from) = @_;
	while($code =~ /^\s+([0-9a-f]+):/mg) {
		return hex($1) if hex($1) > $from;
	}
	return undef;
}

# Of those heads (offset to its last branch), the innermost.
sub innermost_heads
{
	my (%heads) = @_;
	my @all = sort { $a <=> $b } keys %heads;
	return grep {
		my $head = $_;
		!grep { $_ > $head && $_ <= $heads{$head} } @all;
	} @all;
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
		my $code = read_file($disassembly);
		my $body = object_function($test, $code, 'nested');
		my %heads = loop_heads($body);
		die "$test: $level has no two loops to read\n" if scalar(keys %heads) < 2;
		next if $level eq 'O1';
		my $windows = 0;
		for my $head (innermost_heads(%heads)) {
			die sprintf("%s: %s loop head at %x is off its boundary\n",
				$test, $level, $head) if $head % 8 != 0;
			my $end = instruction_end($code, $heads{$head});
			die "$test: $level loop at $head has no end\n" if !defined($end);
			next if $end - $head > 32;
			++$windows;
			die sprintf("%s: %s loop at %x crosses a 32-byte window\n",
				$test, $level, $head) if $head % 32 + ($end - $head) > 32;
		}
		die "$test: $level has no short loop to read\n" if !$windows;
		my $headers = "$directory/$level.headers";
		$status = run_command_capture(
			cmd => ['objdump', '-h', $object],
			stdout => $headers,
			stderr => "$headers.stderr",
			timeout => 30,
		);
		die "$test: $level section headers failed\n" if $status != 0;
		die "$test: $level text section is not aligned to 32\n"
			if read_file($headers) !~ /^\s*\d+\s+\.text\s.*\s2\*\*(\d+)\s*$/m || $1 < 5;
	}
}

print "host loop alignment: PASS (" . scalar(@tests) . "/" .
	scalar(@tests) . ")\n" unless $ENV{CPPGM_REPORT_QUIET};
