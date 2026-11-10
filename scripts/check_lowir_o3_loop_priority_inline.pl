#!/usr/bin/perl

use strict;
use warnings;

use File::Temp qw(tempdir);
use FindBin;
use lib $FindBin::Bin;

use CppgmBatchWorker qw(collect_tests run_command_capture);

sub read_file
{
	my ($path) = @_;
	open(my $fh, '<', $path) or die "Unable to read $path: $!\n";
	local $/;
	my $data = <$fh>;
	close($fh) or die "Unable to close $path: $!\n";
	return defined($data) ? $data : '';
}

sub optimize
{
	my ($app, $test, $directory, $level) = @_;
	my $output = "$directory/$level.lowir";
	my $status = run_command_capture(
		cmd => [$app, "-$level", '-o', $output, $test],
		stdout => "$directory/$level.stdout",
		stderr => "$directory/$level.stderr",
		timeout => 30,
	);
	die "$test: lowiropt -$level failed\n" .
		read_file("$directory/$level.stderr") if $status != 0;
	return ($output, read_file($output));
}

sub function_body
{
	my ($test, $lowir, $name) = @_;
	return $1 if $lowir =~
		/(function \@\Q$name\E\b.*?)(?=\nfunction \@|\z)/s;
	die "$test: optimized output has no $name definition\n";
}

sub call_count
{
	my ($body, $callee) = @_;
	return scalar(() = $body =~ /call i64 \@\Q$callee\E\(/g);
}

sub require_pair_calls
{
	my ($test, $lowir, $function, $expected, $level) = @_;
	my $body = function_body($test, $lowir, $function);
	my $actual = call_count($body, 'preferred_body');
	die "$test: $level $function has $actual preferred-body calls, " .
		"expected $expected\n" if $actual != $expected;
	return $body;
}

sub compile_and_run
{
	my ($driver, $test, $directory, $lowir) = @_;
	my $program = "$directory/behavior";
	my $status = run_command_capture(
		cmd => [$driver, '-O0', '-o', $program, $lowir],
		stdout => "$directory/compile.stdout",
		stderr => "$directory/compile.stderr",
		timeout => 60,
	);
	die "$test: optimized LowIR did not compile\n" .
		read_file("$directory/compile.stderr") if $status != 0;
	$status = run_command_capture(
		cmd => [$program],
		stdout => "$directory/run.stdout",
		stderr => "$directory/run.stderr",
		timeout => 30,
	);
	die "$test: optimized behavior failed with status $status\n"
		if $status != 0;
}

if(scalar(@ARGV) != 3)
{
	die "Usage: check_lowir_o3_loop_priority_inline.pl " .
		"<lowiropt> <cppgm++> <test-or-directory>\n";
}

my ($app, $driver, $root) = @ARGV;
my @tests = collect_tests($root, qr/o3-loop-priority-inline\.t$/);
die "No O3 loop-priority inline controls found under $root\n" if !@tests;

for my $test (@tests)
{
	my $directory = tempdir('lowir-o3-loop-priority-inline-XXXXXX',
		TMPDIR => 1, CLEANUP => 1);
	my (undef, $o0) = optimize($app, $test, $directory, 'O0');
	for my $caller (qw(loop_first loop_second no_loop_pair two_loop_calls))
	{
		require_pair_calls($test, $o0, $caller, 2, 'O0');
	}
	# Large shared bodies must not multiply merely because a call is in a
	# loop. Bound code growth without requiring a particular call decision.
	my $input_size = scalar(() = $o0 =~ /^    \S/gm);
	for my $level (qw(O1 O2 O3))
	{
		my ($path, $lowir) = optimize($app, $test, $directory, $level);
		my $output_size = scalar(() = $lowir =~ /^    \S/gm);
		die "$test: $level expanded the shared bodies from $input_size " .
			"to $output_size instructions\n" if $output_size > $input_size + 32;
		for my $caller (qw(loop_first loop_second no_loop_pair two_loop_calls))
		{
			my $body = function_body($test, $lowir, $caller);
			my $size = scalar(() = $body =~ /^    \S/gm);
			die "$test: $level $caller exceeds its 64-instruction envelope\n"
				if $size > 64;
		}
		# Inlined or retained, a noreturn arm has no normal successor and
		# contributes no incoming value to a merge phi in its function.
		while ($lowir =~ /(function \@[^\n]+.*?)(?=\nfunction \@|\z)/sg)
		{
			my $body = $1;
			while ($body =~ /^\s*block \^([^:]+):\n(.*?)(?=^\s*block \^|\z)/msg)
			{
				my ($label, $block) = ($1, $2);
				next if $block !~ /^\s+call void \@fail\(\)$/m;
				die "$test: $level retained a normal successor after noreturn\n"
					if $block =~ /^\s+(?:jump|branch|switch)\b/m;
				die "$test: $level noreturn successor remains in a merge phi\n"
					if $body =~ /^\s+%\w+ = phi [^\n]*\[\^\Q$label\E:/m;
			}
		}
		compile_and_run($driver, $test, $directory, $path);
	}
}

print "loop-site code growth and noreturn safety: PASS (" . scalar(@tests) . "/" .
	scalar(@tests) . ")\n" unless $ENV{CPPGM_REPORT_QUIET};
