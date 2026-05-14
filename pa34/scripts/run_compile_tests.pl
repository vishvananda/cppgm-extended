#!/usr/bin/perl

use strict;
use warnings;
use File::Find;

sub detect_jobs
{
	my $jobs = $ENV{CPPGM_TEST_JOBS};
	if (defined($jobs) && $jobs =~ m/^\d+$/ && $jobs > 0)
	{
		return $jobs;
	}
	chomp($jobs = `getconf _NPROCESSORS_ONLN 2>/dev/null`);
	if ($jobs =~ m/^\d+$/ && $jobs > 0)
	{
		return $jobs;
	}
	return 1;
}

sub collect_tests
{
	my ($root, $pattern) = @_;
	my @tests;
	find(sub {
		return if !-f $_;
		push @tests, $File::Find::name if $File::Find::name =~ $pattern;
	}, $root);
	return sort @tests;
}

sub write_named_status_code
{
	my ($path, $status) = @_;
	open(my $fh, '>', $path) or die "Unable to write $path: $!";
	if (!defined($status))
	{
		print $fh "EXIT_FAILURE\n";
	}
	elsif ($status == 0)
	{
		print $fh "EXIT_SUCCESS\n";
	}
	elsif ($status == 124)
	{
		print $fh "EXIT_TIMEOUT\n";
	}
	elsif ($status == 86)
	{
		print $fh "EXIT_NOT_IMPLEMENTED\n";
	}
	else
	{
		print $fh "EXIT_FAILURE\n";
	}
	close($fh) or die "Unable to close $path: $!";
}

sub system_status_to_exit_code
{
	my ($status) = @_;
	return 1 if !defined($status) || $status == -1;
	return $status >> 8 if ($status & 127) == 0;
	return 128 + ($status & 127);
}

sub run_tests
{
	my ($tests, $jobs, $runner) = @_;
	if ($jobs <= 1)
	{
		for my $test (@{$tests})
		{
			print "Running $test...\n";
			$runner->($test);
		}
		return;
	}

	my %children;
	for my $test (@{$tests})
	{
		print "Running $test...\n";
		while (scalar(keys %children) >= $jobs)
		{
			my $pid = waitpid(-1, 0);
			die "waitpid failed: $!" if $pid == -1;
			delete $children{$pid};
		}

		my $pid = fork();
		die "fork failed: $!" unless defined($pid);
		if ($pid == 0)
		{
			$runner->($test);
			exit 0;
		}
		$children{$pid} = 1;
	}

	while (%children)
	{
		my $pid = waitpid(-1, 0);
		die "waitpid failed: $!" if $pid == -1;
		delete $children{$pid};
	}
}

if (scalar(@ARGV) != 3)
{
	die "Usage: run_compile_tests.pl <app> <suffix> <testlocation>";
}

my $app = $ARGV[0];
my $suffix = $ARGV[1];
my $tests = $ARGV[2];
my $jobs = detect_jobs();

my @tests = collect_tests($tests, qr/\.t$/);
run_tests(\@tests, $jobs, sub {
	my ($test) = @_;
	my $test_out = $test;
	$test_out =~ s/\.t$/\.$suffix/;
	my $sys_ret = system("bash", "scripts/run_one_compile_test.sh", $app, $test, $test_out);
	write_named_status_code("$test_out.exit_status", system_status_to_exit_code($sys_ret));
});
