#!/usr/bin/perl

use strict;
use warnings;
use File::Basename qw(dirname);
use File::Spec;
use File::Temp qw(tempdir);
use FindBin;
use JSON::PP qw(decode_json);
use lib $FindBin::Bin;
use CppgmBatchWorker qw(collect_tests run_command_capture);

sub read_file {
    my ($path) = @_;
    open(my $file, '<', $path) or die "Cannot read $path: $!\n";
    local $/;
    my $text = <$file>;
    return defined($text) ? $text : '';
}

sub expectation {
    my ($test) = @_;
    $test =~ s/\.t$/.wasm-expect/;
    return $test;
}

if (@ARGV && $ARGV[0] eq '--list') {
    shift @ARGV;
    print "$_\n" for grep { -f expectation($_) }
        map { collect_tests($_, qr/\.t$/) } @ARGV;
    exit 0;
}

@ARGV == 2 or die "usage: check_wasm.pl <lowir2native> <fixture>\n";
my ($compiler, $test) = @ARGV;
$compiler = File::Spec->rel2abs($compiler);
my $expect = decode_json(read_file(expectation($test)));
my $directory = tempdir('cppgm-wasm-XXXXXX', TMPDIR => 1, CLEANUP => 1);
my $validator = $ENV{CPPGM_WASM_TOOLS} // 'wasm-tools';
my $runtime = $ENV{CPPGM_WASMTIME} // 'wasmtime';
my $source_compiler = $ENV{CPPGM_WASM_CPPGM} // File::Spec->catfile(dirname($compiler),
    $compiler =~ /-ref\z/ ? 'cppgm++-ref' : 'cppgm++');
my $command_sequence = 0;

sub command {
    my ($name, @args) = @_;
    $name .= '-' . ++$command_sequence;
    my $status = run_command_capture(cmd => \@args,
        stdout => "$directory/$name.stdout", stderr => "$directory/$name.stderr", timeout => 30);
    return ($status, read_file("$directory/$name.stdout"), read_file("$directory/$name.stderr"));
}

for my $level (@{$expect->{levels} // [0, 1, 2, 3]}) {
    my $module = "$directory/program-$level.wasm";
    my @exports = map { ('--export', $_->{export}) } @{$expect->{calls} // []};
    my $tool = $expect->{source} ? $source_compiler : $compiler;
    my ($status, $stdout, $stderr) = command("compile-$level", $tool, "-O$level",
        '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
        @exports, '-o', $module, $test);
    if ($expect->{reject}) {
        die "$test: invalid input accepted at O$level\n" if $status == 0;
        die "$test: rejection was not a compiler diagnostic: $stdout$stderr\n"
            if "$stdout$stderr" !~ /ERROR:/ || $status >= 128;
        next;
    }
    die "$test: compile O$level failed ($status): $stdout$stderr\n" if $status;
    ($status, $stdout, $stderr) = command("validate-$level", $validator, 'validate', $module);
    die "$test: invalid module at O$level: $stdout$stderr\n" if $status;
    my @modules = ($module);
    if ($expect->{source}) {
        my $lowir = "$directory/source.lowir";
        ($status, $stdout, $stderr) = command("lowir-$level", $source_compiler,
            '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
            '--emit-lowir', '-O0', '-o', $lowir, $test);
        die "$test: LowIR lowering failed: $stdout$stderr\n" if $status;
        my $roundtrip = "$directory/roundtrip-$level.wasm";
        ($status, $stdout, $stderr) = command("roundtrip-$level", $compiler,
            "-O$level", @exports, '-o', $roundtrip, $lowir);
        die "$test: LowIR backend failed: $stdout$stderr\n" if $status;
        ($status, $stdout, $stderr) = command("roundtrip-validate-$level", $validator, 'validate', $roundtrip);
        die "$test: invalid round-trip module: $stdout$stderr\n" if $status;
        push @modules, $roundtrip;
    }
    for my $run_module (@modules) {
    for my $call (@{$expect->{calls}}) {
        ($status, $stdout, $stderr) = command("run-$level-$call->{export}", $runtime, 'run',
            '--invoke', $call->{export}, $run_module, @{$call->{args} // []});
        $stdout =~ s/\s+\z//;
        die "$test: $call->{export} failed ($status): $stdout$stderr\n" if $status;
        die "$test: $call->{export} expected $call->{result}, got '$stdout'\n"
            if $stdout ne "$call->{result}";
    }
    }
}
print "Wasm behavior: PASS\n";
