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
#
# A scan fixture (`host-scan-cycle-line.t`, C++) holds short cycles that leave
# on the data they read. A processor that keeps decoded code by the 64-byte
# line runs such a cycle from one line, and a cycle on two lines takes a second
# fetch each pass. Each cycle of a `scan` function must lie in one line
# wherever the linker places its section: the unit is the section's alignment,
# 64 bytes at most. The fixture's first line gives the number of cycles.

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

# The output of a tool that must succeed.
sub tool_output
{
	my ($test, $directory, $name, @command) = @_;
	my $status = run_command_capture(
		cmd => \@command,
		stdout => "$directory/$name.stdout",
		stderr => "$directory/$name.stderr",
		timeout => 30,
	);
	die "$test: $command[0] failed for $name\n" if $status != 0;
	return read_file("$directory/$name.stdout");
}

# The cycles of the `scan` functions of an object: [name, start, end, unit].
sub scan_cycles
{
	my ($test, $directory, $level, $object) = @_;
	# Code sections in header order, which is the order of their disassembly
	# (a function of a group has a section of its own, of the same name).
	my (@order, %unit);
	for my $line (split /\n/, tool_output($test, $directory, "$level.sections", 'readelf', '-SW', $object)) {
		next unless $line =~ /^\s*\[\s*(\d+)\]\s+\S+\s+PROGBITS\s+[0-9a-f]+\s+[0-9a-f]+\s+[0-9a-f]+\s+[0-9a-f]+\s+(\S+)\s+\d+\s+\d+\s+(\d+)\s*$/;
		next unless index($2, 'X') >= 0;
		push @order, $1;
		$unit{$1} = $3 < 64 ? $3 : 64;
	}
	my %functions;
	for my $line (split /\n/, tool_output($test, $directory, "$level.symbols", 'readelf', '-sW', $object)) {
		next unless $line =~ /^\s*\d+:\s+([0-9a-f]+)\s+(\d+)\s+FUNC\s+\S+\s+\S+\s+(\d+)\s+(\S*scan\S*)$/;
		push @{$functions{$3}}, [$4, hex($1), hex($1) + $2];
	}
	my ($block, @code, @cycles) = (-1);
	my $finish = sub {
		return if $block < 0 || !defined $order[$block] || !$functions{$order[$block]};
		for my $function (@{$functions{$order[$block]}}) {
			my ($name, $first, $past) = @$function;
			for my $k (0 .. $#code) {
				my ($at, $target) = @{$code[$k]};
				next unless defined $target && $at >= $first && $at < $past && $target >= $first && $target <= $at;
				my $end = $k < $#code ? $code[$k + 1][0] : $past;
				push @cycles, [$name, $target, $end, $unit{$order[$block]}] if $end - $target <= 64;
			}
		}
	};
	for my $line (split /\n/, tool_output($test, $directory, "$level.code", 'objdump', '-d', '--no-show-raw-insn', $object)) {
		if ($line =~ /^Disassembly of section /) {
			$finish->();
			++$block;
			@code = ();
		}
		elsif ($line =~ /^\s*([0-9a-f]+):\s+(\S+)\s*(.*)$/) {
			my ($at, $operation, $operands) = (hex($1), $2, $3);
			push @code, [$at, $operation =~ /^j/ && $operands =~ /^([0-9a-f]+) </ ? hex($1) : undef];
		}
	}
	$finish->();
	return @cycles;
}

sub check_scan_fixture
{
	my ($driver, $test) = @_;
	my ($expected) = read_file($test) =~ /\A\/\/ cycles: (\d+)\n/;
	die "$test: the first line must be `// cycles: N`\n" unless defined $expected;
	my $directory = tempdir('native-scan-line-XXXXXX', TMPDIR => 1, CLEANUP => 1);
	my $input = "$directory/input.cpp";
	copy($test, $input) or die "$test: unable to prepare the input: $!\n";
	for my $level (qw(O2 O3)) {
		my $object = "$directory/$level.o";
		my $status = run_command_capture(
			cmd => [$driver, '-std=gnu++17', '-c', "-$level", '-o', $object, $input],
			stdout => "$directory/$level.compile.stdout",
			stderr => "$directory/$level.compile.stderr",
			timeout => 60,
		);
		die "$test: $level object compile failed\n" . read_file("$directory/$level.compile.stderr") if $status != 0;
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
		die "$test: $level behavior failed with status $status\n" if $status != 0;
		my @cycles = scan_cycles($test, $directory, $level, $object);
		die "$test: $level has " . scalar(@cycles) . " scan cycles (expected $expected)\n" if @cycles != $expected;
		for my $cycle (@cycles) {
			my ($name, $start, $end, $unit) = @$cycle;
			next if int($start / $unit) == int(($end - 1) / $unit);
			die sprintf("%s: %s cycle of %s at 0x%x-0x%x may lie on two lines (section alignment %d)\n",
				$test, $level, $name, $start, $end, $unit);
		}
	}
}

my ($driver, $root) = @ARGV;
exit 0 unless CppgmTestPlatform::require_platform(
    'linux', "$FindBin::Script:$root", 'ELF symbol, section and code inspection');
my @scans = collect_tests($root, qr/host-scan-cycle-line\.t$/);
check_scan_fixture($driver, $_) for @scans;
my @tests = collect_tests($root, qr/host-loop-alignment\.t$/);
die "No host loop-alignment controls found under $root\n" if !@tests && !@scans;

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

my $total = scalar(@tests) + scalar(@scans);
print "host loop layout: PASS ($total/$total)\n" unless $ENV{CPPGM_REPORT_QUIET};
