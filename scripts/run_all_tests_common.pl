#!/usr/bin/perl

use strict;
use warnings;
use Cwd qw(abs_path getcwd);
use Errno qw(EACCES EPERM ESRCH);
use File::Basename qw(basename);
use File::Find;
use Fcntl qw(F_GETFD F_SETFD FD_CLOEXEC);
use IPC::Open2;
use POSIX qw(setpgid WNOHANG);
use Text::ParseWords qw(shellwords);

my %wrapped_worker_has_process_group;

sub local_exec_path
{
	my ($path) = @_;
	return $path if $path =~ m{/};
	return "./$path";
}

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

sub get_timeout_from_env
{
	my ($name, $default) = @_;
	my $value = $ENV{$name};
	return $default if !defined($value);
	return $default if $value !~ m/^\d+$/;
	return $value > 0 ? $value : $default;
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

sub write_numeric_status
{
	my ($path, $status) = @_;
	open(my $fh, '>', $path) or die "Unable to write $path: $!";
	print $fh "$status\n";
	close($fh) or die "Unable to close $path: $!";
}

sub system_status_to_exit_code
{
	my ($status) = @_;
	return 1 if !defined($status) || $status == -1;
	return $status >> 8 if ($status & 127) == 0;
	return 128 + ($status & 127);
}

sub write_file
{
	my ($path, $contents) = @_;
	open(my $fh, '>', $path) or die "Unable to write $path: $!";
	print $fh $contents;
	close($fh) or die "Unable to close $path: $!";
}

sub encode_env
{
	my ($env) = @_;
	return '' if !defined($env);
	return join(';', map { $_ . '=' . $env->{$_} } sort keys %{$env});
}

sub read_env_file
{
	my ($path) = @_;
	my %env;
	return \%env if !-f $path;

	open(my $fh, '<', $path) or die "Unable to read $path: $!";
	while(my $line = <$fh>)
	{
		chomp($line);
		next if $line =~ m/^\s*$/;
		next if $line =~ m/^\s*#/;
		if ($line =~ m/^([A-Za-z_][A-Za-z0-9_]*)=(.*)$/)
		{
			$env{$1} = $2;
		}
	}
	close($fh) or die "Unable to close $path: $!";
	return \%env;
}

sub open_wrapped_worker
{
	my ($app) = @_;
	my @app_args = shellwords($ENV{CPPGM_APP_ARGS} || '');
	$app = local_exec_path($app);
	my ($worker_out, $worker_in);
	my $pid = open2($worker_out,
	               $worker_in,
	               'env',
	               'WRAPPED_BATCH_STDIN=1',
	               $app,
	               @app_args,
	               '--batch-stdin');
	select((select($worker_in), $| = 1)[0]);
	set_cloexec($worker_in, "wrapped worker stdin");
	set_cloexec($worker_out, "wrapped worker stdout");
	$wrapped_worker_has_process_group{$pid} = try_set_process_group($pid);
	return ($pid, $worker_out, $worker_in);
}

sub try_set_process_group
{
	my ($pid) = @_;
	return 0 if !defined($pid) || $pid <= 0;
	return 1 if setpgid($pid, $pid);
	return 0 if $!{EACCES} || $!{EPERM} || $!{ESRCH};
	die "Unable to set wrapped worker process group for $pid: $!";
}

sub set_cloexec
{
	my ($fh, $label) = @_;
	my $flags = fcntl($fh, F_GETFD, 0);
	die "Unable to read close-on-exec flags for $label: $!" if !defined($flags);
	fcntl($fh, F_SETFD, $flags | FD_CLOEXEC)
		or die "Unable to set close-on-exec for $label: $!";
}

sub close_wrapped_worker
{
	my ($pid, $worker_out, $worker_in) = @_;
	close($worker_in) or die "Unable to close worker stdin: $!";
	close($worker_out) or die "Unable to close worker stdout: $!";
	if (wait_for_pid_exit($pid, 500))
	{
		delete $wrapped_worker_has_process_group{$pid};
		die "worker exited with status $?\n" if $? != 0;
		return;
	}
	force_terminate_wrapped_worker($pid);
}

sub wait_for_pid_exit
{
	my ($pid, $timeout_ms) = @_;
	while ($timeout_ms > 0)
	{
		my $res = waitpid($pid, WNOHANG);
		return 1 if $res == $pid;
		return 0 if $res < 0;
		select(undef, undef, undef, 0.01);
		$timeout_ms -= 10;
	}
	return 0;
}

sub force_terminate_wrapped_worker
{
	my ($pid) = @_;
	my $has_process_group = delete $wrapped_worker_has_process_group{$pid};
	kill 'TERM', $pid;
	kill 'TERM', -$pid if $has_process_group;
	if (!wait_for_pid_exit($pid, 500))
	{
		kill 'KILL', $pid;
		kill 'KILL', -$pid if $has_process_group;
		waitpid($pid, 0);
	}
}

sub submit_wrapped_request
{
	my ($worker_in, $worker_out, $stdout_path, $stderr_path, $stdin_path, $env, @args) = @_;
	$stdin_path = "-" if !defined($stdin_path) || $stdin_path eq '';
	print {$worker_in} join("\t",
	                       $stdout_path,
	                       $stderr_path,
	                       $stdin_path,
	                       encode_env($env),
	                       @args), "\n"
		or die "Unable to write work item: $!";
	my $status = <$worker_out>;
	die "worker exited unexpectedly\n" if !defined($status);
	chomp($status);
	return 0 if $status eq 'EXIT_SUCCESS';
	return 124 if $status eq 'EXIT_TIMEOUT';
	return 86 if $status eq 'EXIT_NOT_IMPLEMENTED';
	return 1 if $status eq 'EXIT_FAILURE';
	return $status if $status =~ m/^\d+$/;
	die "Unexpected worker status '$status'\n";
}

sub run_command_capture
{
	my (%options) = @_;
	my @cmd = @{$options{cmd}};
	my $stdout_path = $options{stdout};
	my $stderr_path = $options{stderr};
	my $stdin_path = $options{stdin};
	my $timeout = $options{timeout};

	open(my $stdout_fh, '>', $stdout_path)
		or die "Unable to write $stdout_path: $!";
	open(my $stderr_fh, '>', $stderr_path)
		or die "Unable to write $stderr_path: $!";

	my $pid = fork();
	die "fork failed: $!" unless defined($pid);
	if ($pid == 0)
	{
		setpgid(0, 0);
		if (defined($stdin_path))
		{
			open(STDIN, '<', $stdin_path)
				or die "Unable to read $stdin_path: $!";
		}
		open(STDOUT, '>&', $stdout_fh)
			or die "Unable to redirect stdout: $!";
		open(STDERR, '>&', $stderr_fh)
			or die "Unable to redirect stderr: $!";
		exec {$cmd[0]} @cmd;
		die "Unable to exec $cmd[0]: $!";
	}
	my $has_process_group = try_set_process_group($pid);

	if (defined($timeout))
	{
		my $timeout_ms = $timeout > 2147483 ? 2147483000 : $timeout * 1000;
		if (!wait_for_pid_exit($pid, $timeout_ms))
		{
			kill 'TERM', $pid;
			kill 'TERM', -$pid if $has_process_group;
			if (!wait_for_pid_exit($pid, 200))
			{
				kill 'KILL', $pid;
				kill 'KILL', -$pid if $has_process_group;
				waitpid($pid, 0);
			}
			close($stdout_fh) or die "Unable to close $stdout_path: $!";
			close($stderr_fh) or die "Unable to close $stderr_path: $!";
			return 124;
		}
	}
	else
	{
		waitpid($pid, 0);
	}

	close($stdout_fh) or die "Unable to close $stdout_path: $!";
	close($stderr_fh) or die "Unable to close $stderr_path: $!";

	return $? >> 8 if ($? & 127) == 0;
	return 128 + ($? & 127);
}

sub sorted_glob
{
	my ($pattern) = @_;
	return sort glob($pattern);
}

sub build_wrapped_text_request
{
	my ($mode, $suffix, $test, $assignment) = @_;
	if ($mode eq "text_t")
	{
		my $test_out = $test;
		$test_out =~ s/\.t$/\.$suffix/;

		if ($assignment =~ m/^pa[1-4]$/)
		{
			my $stderr_file = ($assignment eq 'pa1') ? $test_out : "$test_out.stderr";
			return ($test_out, $stderr_file, $test, {});
		}

		if ($assignment eq 'pa5')
		{
			my $test_base = $test;
			$test_base =~ s/\.t$//;
			my @inputs = sorted_glob("$test*");
			return ("$test_out.stdout",
			        "$test_out.stdout",
			        "-",
			        read_env_file("$test_base.env"),
			        "-o",
			        $test_out,
			        @inputs);
		}

		return ("$test_out.stdout",
		        "$test_out.stdout",
		        "-",
		        {},
		        "-o",
		        $test_out,
		        $test);
	}

	if ($mode eq "witness_t")
	{
		my $test_out = $test;
		$test_out =~ s/\.t$/\.$suffix/;
		my $test_input = abs_path($test) || $test;
		return ("$test_out.stdout",
		        "$test_out.stderr",
		        "-",
		        {},
		        "-o",
		        $test_out,
		        "--witness",
		        "$test_out.witness",
		        $test_input);
	}

	if ($mode eq "text_t1")
	{
		my $test_out = $test;
		$test_out =~ s/\.t\.1$/\.$suffix/;
		my $test_base = $test;
		$test_base =~ s/\.t\.1$/\.t/;
		my @inputs = sorted_glob("$test_base.*");
		return ("$test_out.stdout",
		        "$test_out.stdout",
		        "-",
		        {},
		        "-o",
		        $test_out,
		        @inputs);
	}

	die "Unsupported wrapped text mode $mode";
}

sub run_tests
{
	my ($tests, $jobs, $runner, $verbose) = @_;
	if ($jobs <= 1)
	{
		for my $test (@{$tests})
		{
			print "Running $test...\n" if $verbose;
			$runner->($test);
		}
		return;
	}

	my %children;
	for my $test (@{$tests})
	{
		print "Running $test...\n" if $verbose;
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

sub run_batch_wrapped_text
{
	my ($mode, $app, $suffix, $tests, $verbose, $assignment) = @_;
	my ($worker_pid, $worker_out, $worker_in) = open_wrapped_worker($app);
	for my $test (@{$tests})
	{
		print "Running $test...\n" if $verbose;
		my $test_out = $test;
		$test_out =~ s/\.t(\.1)?$/\.$suffix/;
		my ($stdout_path, $stderr_path, $stdin_path, $env, @args) =
			build_wrapped_text_request($mode, $suffix, $test, $assignment);
		my $status = submit_wrapped_request(
			$worker_in,
			$worker_out,
			$stdout_path,
			$stderr_path,
			$stdin_path,
			$env,
			@args);
		write_named_status_code("$test_out.exit_status", $status);
	}
	close_wrapped_worker($worker_pid, $worker_out, $worker_in);
}

sub run_batch_wrapped_pa9
{
	my ($app, $suffix, $tests, $verbose) = @_;
	my ($worker_pid, $worker_out, $worker_in) = open_wrapped_worker($app);
	my $build_timeout = get_timeout_from_env("CPPGM_BUILD_TEST_TIMEOUT_SEC", 30);

	for my $test (@{$tests})
	{
		print "Running $test...\n" if $verbose;
		my $test_base = $test;
		$test_base =~ s/\.t\.1$//;

		unlink(glob("$test_base.$suffix.program"));
		unlink(glob("$test_base.$suffix.program.exit_status"));
		unlink(glob("$test_base.$suffix.program.stdout"));
		unlink(glob("$test_base.$suffix.program.stderr"));
		unlink(glob("$test_base.$suffix.impl.stdout"));
		unlink(glob("$test_base.$suffix.impl.stderr"));
		unlink(glob("$test_base.$suffix.impl.exit_status"));

		my @srcfiles = sorted_glob("$test_base.t.*");
		my @args;
		if (defined($ENV{CY86_TARGET}) && $ENV{CY86_TARGET} ne '')
		{
			push @args, '--target', $ENV{CY86_TARGET};
		}
		push @args, '-o', "$test_base.$suffix.program", @srcfiles;

		my $impl_status = submit_wrapped_request(
			$worker_in,
			$worker_out,
			"$test_base.$suffix.impl.stdout",
			"$test_base.$suffix.impl.stderr",
			"-",
			{ CPPGM_BATCH_TIMEOUT_SEC => $build_timeout },
			@args);
		write_numeric_status("$test_base.$suffix.impl.exit_status", $impl_status);

		if ($impl_status == 0)
		{
				my $program_status = run_command_capture(
					cmd => [local_exec_path("$test_base.$suffix.program")],
					stdout => "$test_base.$suffix.program.stdout",
					stderr => "$test_base.$suffix.program.stderr",
					stdin => "$test_base.stdin",
					timeout => get_timeout_from_env("CPPGM_PROGRAM_TEST_TIMEOUT_SEC", 10),
				);
			write_numeric_status("$test_base.$suffix.program.exit_status", $program_status);
		}
	}

	close_wrapped_worker($worker_pid, $worker_out, $worker_in);
}

sub shard_tests
{
	my ($tests, $jobs) = @_;
	$jobs = scalar(@{$tests}) if $jobs > scalar(@{$tests});
	my @shards;
	for (my $i = 0; $i < $jobs; ++$i)
	{
		$shards[$i] = [];
	}
	for (my $i = 0; $i < scalar(@{$tests}); ++$i)
	{
		push @{$shards[$i % $jobs]}, $tests->[$i];
	}
	return @shards;
}

sub run_batch_sharded
{
	my ($runner, $tests, $jobs) = @_;
	if ($jobs <= 1 || scalar(@{$tests}) <= 1)
	{
		$runner->($tests);
		return;
	}

	my @shards = shard_tests($tests, $jobs);
	my @pids;
	for my $shard (@shards)
	{
		next if scalar(@{$shard}) == 0;
		my $pid = fork();
		die "fork failed: $!" unless defined($pid);
		if ($pid == 0)
		{
			$runner->($shard);
			exit 0;
		}
		push @pids, $pid;
	}

	my $failed = 0;
	for my $pid (@pids)
	{
		waitpid($pid, 0);
		$failed = 1 if $? != 0;
	}
	die "batched test worker failed\n" if $failed;
}

sub run_batch
{
	my ($mode, $app, $suffix, $tests, $jobs, $verbose, $assignment) = @_;
	if ($mode eq "text_t" || $mode eq "text_t1" || $mode eq "witness_t")
	{
		run_batch_sharded(sub {
			my ($shard) = @_;
			run_batch_wrapped_text($mode, $app, $suffix, $shard, $verbose, $assignment);
		}, $tests, $jobs);
		return;
	}
	if ($mode eq "driver_t1")
	{
		run_batch_sharded(sub {
			my ($shard) = @_;
			run_batch_wrapped_pa9($app, $suffix, $shard, $verbose);
		}, $tests, $jobs);
		return;
	}
	die "Unsupported wrapped batch mode $mode";
}

sub run_single
{
	my ($mode, $app, $suffix, $tests, $jobs, $verbose) = @_;
	run_tests($tests, $jobs, sub {
		my ($test) = @_;
		if ($mode eq "text_t")
		{
			my $test_out = $test;
			$test_out =~ s/\.t$/\.$suffix/;
			my $sys_ret = system("scripts/run_one_test.sh", $app, $test, $test_out);
			write_named_status_code("$test_out.exit_status", system_status_to_exit_code($sys_ret));
			return;
		}
		if ($mode eq "witness_t")
		{
			my $test_out = $test;
			$test_out =~ s/\.t$/\.$suffix/;
			my $test_input = abs_path($test) || $test;
			my @app_args = shellwords($ENV{CPPGM_APP_ARGS} || '');
			my $status = run_command_capture(
				cmd => [local_exec_path($app),
				        @app_args,
				        "-o",
				        $test_out,
				        "--witness",
				        "$test_out.witness",
				        $test_input],
				stdout => "$test_out.stdout",
				stderr => "$test_out.stderr",
				timeout => get_timeout_from_env("CPPGM_TEXT_TEST_TIMEOUT_SEC", 10),
			);
			write_named_status_code("$test_out.exit_status", $status);
			return;
		}
		if ($mode eq "text_t1")
		{
			my $test_out = $test;
			$test_out =~ s/\.t\.1$/\.$suffix/;
			my $test_base = $test;
			$test_base =~ s/\.t\.1$/\.t/;
			my $sys_ret = system("scripts/run_one_test.sh", $app, $test_out, $test_base);
			write_named_status_code("$test_out.exit_status", system_status_to_exit_code($sys_ret));
			return;
		}
		if ($mode eq "driver_t")
		{
			my $test_base = $test;
			$test_base =~ s/\.t$//;
			system("scripts/run_one_test.sh", $app, $suffix, $test_base);
			return;
		}
		if ($mode eq "driver_t1")
		{
			my $test_base = $test;
			$test_base =~ s/\.t\.1$//;
			system("scripts/run_one_test.sh", $app, $suffix, $test_base);
			return;
		}
		die "Unsupported run_all_tests mode $mode";
	}, $verbose);
}

if (scalar(@ARGV) != 4)
{
	die "Usage: run_all_tests_common.pl <mode> <app> <suffix> <testlocation>";
}

my $mode = shift(@ARGV);
my $app = $ARGV[0];
my $suffix = $ARGV[1];
my $tests = $ARGV[2];
my $verbose = $ENV{VERBOSE} || $ENV{CPGM_TEST_VERBOSE};
my $keep_going = $ENV{KEEP_GOING};
my $jobs = detect_jobs();
my $assignment = basename(getcwd());

my %patterns = (
	text_t => qr/\.t$/,
	witness_t => qr/\.t$/,
	text_t1 => qr/\.t\.1$/,
	driver_t => qr/\.t$/,
	driver_t1 => qr/\.t\.1$/,
);
die "Unsupported run_all_tests mode $mode" if !exists($patterns{$mode});

my @tests = collect_tests($tests, $patterns{$mode});
my $ntests = scalar(@tests);
if (!$verbose && !$keep_going)
{
	print "$assignment $tests: running $ntests test";
	print "s" if $ntests != 1;
	print "\n";
}

if ($ENV{CPPGM_BATCH_TESTS})
{
	run_batch($mode, $app, $suffix, \@tests, $jobs, $verbose, $assignment);
}
else
{
	run_single($mode, $app, $suffix, \@tests, $jobs, $verbose);
}
