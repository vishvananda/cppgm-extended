#!/usr/bin/perl

use strict;
use warnings;

use Cwd qw(abs_path);
use File::Basename qw(basename dirname);
use File::Copy qw(copy);
use File::Path qw(make_path);
use File::Spec;
use File::Temp qw(tempdir);
use FindBin;
use Text::ParseWords qw(shellwords);
use lib $FindBin::Bin;

use CppgmBatchWorker qw(collect_tests run_command_capture ensure_test_app_available
	get_timeout_from_env note_progress_state clear_progress_state);

sub read_file
{
	my ($path) = @_;
	open(my $fh, '<', $path) or die "Unable to read $path: $!\n";
	binmode($fh);
	local $/;
	my $text = <$fh>;
	close($fh) or die "Unable to close $path: $!\n";
	return defined($text) ? $text : '';
}

sub profile_path
{
	my ($test) = @_;
	$test =~ s/\.t$/.assembly-profile/;
	return $test;
}

# The sidecar identifies this control independently of its assignment number
# or directory. The common runner owns discovery, parallelism and reporting.
if (@ARGV && $ARGV[0] eq '--list')
{
	shift @ARGV;
	print "$_\n" for grep { -f profile_path($_) }
		map { collect_tests($_, qr/\.t$/) } @ARGV;
	exit 0;
}

@ARGV == 2 or die "Usage: check_inline_assembly.pl <cppgm++> <test-or-directory>\n";
my ($app, $selection) = @ARGV;
$app = File::Spec->rel2abs($app); # preserve a reference wrapper's basename
my $lowir = $ENV{CPPGM_ASSEMBLY_LOWIR_APP} // '../dev/lowir';
$lowir = File::Spec->rel2abs($lowir);
my $native = File::Spec->rel2abs($ENV{CPPGM_ASSEMBLY_NATIVE_APP} // '../dev/lowir2native');
my @host = shellwords($ENV{CPPGM_HOST_CXX} // $ENV{CXX} // 'c++');
my @stdlib = shellwords($ENV{CPPGM_STDLIB_FLAGS} // '');
my @levels = split /,/, ($ENV{CPPGM_ASSEMBLY_LEVELS} // 'O0,O3');
die "Invalid assembly optimization levels\n" if !@levels || grep { !/^O[0-3]$/ } @levels;
my @tests = collect_tests($selection, qr/\.t$/);
die "No assembly fixtures found under $selection\n" if !@tests;
ensure_test_app_available($app, 'my', $selection);
my $out = $ENV{CPPGM_ASSEMBLY_OUT} // 'obj/assembly';
make_path($out);

sub command
{
	my ($directory, $step, @argv) = @_;
	my $status = run_command_capture(
		cmd => \@argv, stdout => "$directory/$step.stdout", stderr => "$directory/$step.stderr",
		timeout => get_timeout_from_env('CPPGM_BUILD_TEST_TIMEOUT_SEC', 30));
	die "$step failed with status $status; results: $directory\n" .
		read_file("$directory/$step.stdout") . read_file("$directory/$step.stderr") if $status != 0;
	return read_file("$directory/$step.stdout");
}

# Absolute text addresses require an explicitly non-PIE executable. The fixture
# declares the linkage requirement; ordinary fixtures retain the host default.
my @fixture_link;
sub fixture_link_options
{
	my ($test) = @_;
	$test =~ s/\.t$/.assembly-link-mode/;
	return () if !-f $test;
	my $mode = read_file($test);
	$mode =~ s/\s+\z//;
	die "$test: unknown assembly link mode '$mode'\n" if $mode ne 'no-pie';
	return $^O eq 'darwin' ? ('-Wl,-no_pie') : ('-no-pie');
}

sub requested_instructions
{
    my ($source) = @_;
    (my $path = $source) =~ s/\.t$/.assembly-instructions/;
    my @names = basename($source) =~ /stream/ ? ('movntps') : ();
    if (-f $path) {
        for my $name (split /\n/, read_file($path)) {
            $name =~ s/^\s+|\s+$//g;
            next if $name eq '';
            die "$path: invalid instruction name '$name'\n" if $name !~ /\A[a-z][a-z0-9]*\z/;
            push @names, $name;
        }
    }
    return @names;
}

sub canonical_inputs
{
	my ($directory, $source) = @_;
	my $original = "$directory/original.lowir";
	copy($source, $original) or die "Unable to copy $source: $!\n";
	command($directory, 'roundtrip-first', $lowir, '-o', "$directory/first.lowir", $source);
	command($directory, 'roundtrip-second', $lowir, '-o', "$directory/second.lowir", "$directory/first.lowir");
	die "$source: unstable LowIR roundtrip\n"
		if read_file("$directory/first.lowir") ne read_file("$directory/second.lowir");
	return ($original, "$directory/first.lowir");
}

sub observe
{
	my ($directory, $label, $object, $driver) = @_;
	my $program = "$directory/$label.program";
	command($directory, "$label.link", @host, '-std=c++11', '-O2', @stdlib, @fixture_link,
		$driver, $object, '-o', $program);
	return command($directory, "$label.run", $program);
}

for my $test (@tests)
{
	my $source = abs_path($test);
	@fixture_link = fixture_link_options($test);
	my $profile = read_file(profile_path($test));
	$profile =~ s/\s+\z//;
	die "$test: unknown assembly profile '$profile'\n" if $profile !~ /\A(?:(?:lowir|gnu)(?:-reject)?|gnu-native|lowir-native(?:-reject)?)\z/;
	my $directory = File::Spec->rel2abs(tempdir('case-XXXXXX', DIR => $out, CLEANUP => 0));
	if ($profile =~ /-reject\z/)
	{
		note_progress_state('reject', $test);
		my $native_source = $source;
		if ($profile eq 'lowir-native-reject')
		{
			$native_source = "$directory/accepted.lowir";
			command($directory, 'accepted-transport', $lowir, '-o', $native_source, $source);
		}
		my @cmd = $profile eq 'lowir-reject'
			? ($lowir, '-o', "$directory/rejected.lowir", $source)
			: ($app, ($profile =~ /^gnu/ ? ('--full-asm') : ()), '-O0', '-c', $native_source, '-o', "$directory/rejected.o");
		my $status = run_command_capture(cmd => \@cmd,
			stdout => "$directory/reject.stdout", stderr => "$directory/reject.stderr",
			timeout => get_timeout_from_env('CPPGM_BUILD_TEST_TIMEOUT_SEC', 30));
		die "$test: expected a normal compiler rejection, got status $status; results: $directory\n"
			if $status != 1;
		print "$test: assembly rejection passed\n" unless $ENV{CPPGM_REPORT_QUIET};
		next;
	}
	if ($profile eq 'lowir-native')
	{
		my $index = 0;
		for my $input (canonical_inputs($directory, $source))
		{
			for my $level (@levels)
			{
				my $program = "$directory/native-$index-$level.program";
				command($directory, "native-$index-$level.compile", $native, "-$level", '-o', $program, $input);
				my $output = command($directory, "native-$index-$level.run", $program);
				die "$test: unexpected native output\n" if $output ne '';
			}
			++$index;
		}
		print "$test: standalone assembly properties passed\n" unless $ENV{CPPGM_REPORT_QUIET};
		next;
	}
	(my $driver = $source) =~ s/\.t$/.driver.cpp/;
	die "$test: missing native observer $driver\n" if !-f $driver;
	my @observations;
	my @inputs = ($source);
	if ($profile eq 'lowir')
	{
		note_progress_state('roundtrip', $test);
		@inputs = canonical_inputs($directory, $source);
	}
	my $index = 0;
	for my $input (@inputs)
	{
		for my $level (@levels)
		{
			note_progress_state('compile', $test);
			my $label = "input-$index-$level";
			my $object = "$directory/$label.o";
			my @flags = $profile =~ /\Agnu/ ? ('--full-asm') : ();
			command($directory, "$label.compile", $app, @flags, "-$level", '-c', $input, '-o', $object);
			push @observations, observe($directory, $label, $object, $driver);
			if ($profile =~ /\Agnu/)
			{
				my $lowered = "$directory/$label.lowir";
				my $canonical = "$directory/$label.canonical.lowir";
				command($directory, "$label.lower", $app, '--full-asm', '--emit-lowir', "-$level", '-o', $lowered, $source);
				command($directory, "$label.read", $lowir, '-o', $canonical, $lowered);
				command($directory, "$label.roundtrip-compile", $app, "-$level", '-c', $canonical, '-o', "$directory/$label.roundtrip.o");
				push @observations, observe($directory, "$label.roundtrip", "$directory/$label.roundtrip.o", $driver);
			}
			if (my @requested = requested_instructions($source))
			{
				my @inspect = $^O eq 'darwin' ? ('otool', '-tvV') : ('objdump', '-d');
				my $dump = command($directory, "$label.inspect", @inspect, $object);
                for my $instruction (@requested) {
                    my $spelling = $instruction eq 'int1' ? '(?:int1|icebp)' : quotemeta($instruction);
                    # Apple's disassembler renders the one-byte ICEBP opcode
                    # as data. Require that exact decoded byte in this case.
                    my $undecoded_int1 = $instruction eq 'int1' && $dump =~ /\.byte\s+0xf1\b/i;
                    die "$test: native object lacks the requested $instruction instruction\n"
                        if !$undecoded_int1 && $dump !~ /\b$spelling\b/i;
                }
			}
		}
		++$index;
	}
	if ($profile eq 'gnu')
	{
		command($directory, 'host-compile', @host, '-std=c++11', '-O3', @stdlib, @fixture_link,
			'-x', 'c++', $source, $driver, '-o', "$directory/host.program");
		push @observations, command($directory, 'host-run', "$directory/host.program");
	}
	for my $output (@observations)
	{
		die "$test: native, roundtrip or host observations differ; results: $directory\n"
			if $output ne $observations[0];
	}
	print "$test: assembly properties passed\n" unless $ENV{CPPGM_REPORT_QUIET};
}
clear_progress_state();
