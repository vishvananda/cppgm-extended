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
use CppgmWasmModule qw(export_signatures);

sub read_file {
    my ($path) = @_;
    open(my $file, '<', $path) or die "Cannot read $path: $!\n";
    local $/;
    my $text = <$file>;
    return defined($text) ? $text : '';
}

sub write_file {
    my ($path, $text) = @_;
    open(my $file, '>', $path) or die "Cannot write $path: $!\n";
    print {$file} $text;
    close($file) or die "Cannot close $path: $!\n";
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
my @units = map { File::Spec->catfile(dirname($test), $_) } @{$expect->{units} // []};

sub command {
    my ($name, @args) = @_;
    $name .= '-' . ++$command_sequence;
    my $status = run_command_capture(cmd => \@args,
        stdout => "$directory/$name.stdout", stderr => "$directory/$name.stderr", timeout => 30);
    return ($status, read_file("$directory/$name.stdout"), read_file("$directory/$name.stderr"));
}

sub archive_inputs {
    my ($path, @objects) = @_;
    my $main = shift @objects;
    open(my $file, '>:raw', $path) or die "Cannot write $path: $!\n";
    print {$file} "!<arch>\n";
    for my $index (0 .. $#objects) {
        my $bytes = read_file($objects[$index]);
        print {$file} sprintf('%-16s%-12s%-6s%-6s%-8s%-10s',
            "unit$index.o/", 0, 0, 0, 100644, length($bytes)), "`\n", $bytes;
        print {$file} "\n" if length($bytes) % 2;
    }
    close($file) or die "Cannot close $path: $!\n";
    return ($main, $path);
}

my @runtime_libraries;
for my $library (@{$expect->{libraries} // []}) {
    my $clang = $ENV{CPPGM_WASM_CLANG} // 'wasm-ref-clang++';
    my ($status, $stdout, $stderr) = command('library-path', $clang, '--target=wasm32-wasip1',
        "-print-file-name=lib$library.a");
    $stdout =~ s/\s+\z//;
    die "$test: supplied library lookup failed: $stdout$stderr\n" if $status || !-f $stdout;
    push @runtime_libraries, $stdout;
}
if ($expect->{runtime}) {
    my $clang = $ENV{CPPGM_WASM_CLANG} // 'wasm-ref-clang++';
    for my $query ('--print-libgcc-file-name', '-print-file-name=libc.a') {
        my ($status, $stdout, $stderr) = command('runtime-path', $clang, '--target=wasm32-wasip1', $query);
        $stdout =~ s/\s+\z//;
        die "$test: pinned runtime library lookup failed: $stdout$stderr\n" if $status || !-f $stdout;
        push @runtime_libraries, $stdout;
    }
}

my @target_flags;
if ($expect->{wasi} || $expect->{resource_headers}) {
    my $clang = $ENV{CPPGM_WASM_CLANG} // 'wasm-ref-clang++';
    if ($expect->{wasi}) {
        die "$test: WASI SDK checks require the wasm32-wasip1 target\n"
            unless ($expect->{target} // '') eq 'wasm32-wasip1';
        my ($status, $library, $stderr) = command('wasi-sysroot', $clang,
            '--target=wasm32-wasip1', '-print-file-name=libc.a');
        $library =~ s/\s+\z//;
        die "$test: WASI libc lookup failed: $library$stderr\n" if $status || !-f $library;
        my $sysroot = dirname(dirname(dirname($library)));
        push @target_flags, '--sysroot', $sysroot;
    }
    my ($status, $resource, $stderr) = command('target-resource', $clang, '-print-resource-dir');
    $resource =~ s/\s+\z//;
    die "$test: target resource lookup failed: $resource$stderr\n" if $status || !-d $resource;
    push @target_flags, '-resource-dir', $resource;
}

for my $level (@{$expect->{levels} // [0, 1, 2, 3]}) {
    my ($status, $stdout, $stderr);
    my @modules;
    my $module = "$directory/program-$level.wasm";
    my @exports = map { ('--export', $_) }
        ((map { $_->{export} } @{$expect->{calls} // []}), @{$expect->{exports} // []});
    if (!$expect->{archive_units}) {
    my $tool = $expect->{source} ? $source_compiler : $compiler;
    ($status, $stdout, $stderr) = command("compile-$level", $tool, "-O$level",
        '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
        @target_flags, @{$expect->{flags} // []}, @{$expect->{link_flags} // []}, @exports, '-o', $module, $test, @units, @runtime_libraries);
    if ($expect->{reject}) {
        die "$test: invalid input accepted at O$level\n" if $status == 0;
        die "$test: rejection was not a compiler diagnostic: $stdout$stderr\n"
            if "$stdout$stderr" !~ /ERROR:/ || $status >= 128;
        next;
    }
    die "$test: compile O$level failed ($status): $stdout$stderr\n" if $status;
    ($status, $stdout, $stderr) = command("validate-$level", $validator, 'validate', $module);
    die "$test: invalid module at O$level: $stdout$stderr\n" if $status;
    push @modules, $module;
    if ($expect->{source} && !@units) {
        my $lowir = "$directory/source.lowir";
        ($status, $stdout, $stderr) = command("lowir-$level", $source_compiler,
            '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
            @target_flags, @{$expect->{flags} // []}, '--emit-lowir', '-O0', '-o', $lowir, $test, @units);
        die "$test: LowIR lowering failed: $stdout$stderr\n" if $status;
        my $roundtrip = "$directory/roundtrip-$level.wasm";
        ($status, $stdout, $stderr) = command("roundtrip-$level", $compiler,
            "-O$level", @target_flags, @{$expect->{link_flags} // []}, @exports, '-o', $roundtrip, $lowir, @runtime_libraries);
        die "$test: LowIR backend failed: $stdout$stderr\n" if $status;
        ($status, $stdout, $stderr) = command("roundtrip-validate-$level", $validator, 'validate', $roundtrip);
        die "$test: invalid round-trip module: $stdout$stderr\n" if $status;
        push @modules, $roundtrip;
    }
    }
    my @objects;
    my @roundtrip_objects;
    my @inputs = ($test, @units);
    if (!$expect->{source}) {
        my $input = "$directory/input.lowir";
        write_file($input, read_file($test));
        @inputs = ($input);
    }
    for my $index (0 .. $#inputs) {
        my $object = "$directory/input-$index-$level.o";
        ($status, $stdout, $stderr) = command("object-$index-$level", $source_compiler,
            '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
            @target_flags, @{$expect->{flags} // []}, "-O$level", '-c', $inputs[$index], '-o', $object);
        die "$test: object compile failed: $stdout$stderr\n" if $status;
        ($status, $stdout, $stderr) = command("object-validate-$index-$level", $validator, 'validate', $object);
        die "$test: invalid relocatable object: $stdout$stderr\n" if $status;
        push @objects, $object;
        if ($expect->{source} && @units) {
            my $lowir = "$directory/unit-$index.lowir";
            ($status, $stdout, $stderr) = command("unit-lowir-$index-$level", $source_compiler,
                '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
                @target_flags, @{$expect->{flags} // []}, '--emit-lowir', '-O0', $inputs[$index], '-o', $lowir);
            die "$test: unit LowIR lowering failed: $stdout$stderr\n" if $status;
            my $roundtrip = "$directory/unit-$index-$level.o";
            ($status, $stdout, $stderr) = command("unit-roundtrip-$index-$level", $source_compiler,
                '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
                "-O$level", '-c', $lowir, '-o', $roundtrip);
            die "$test: unit LowIR object compile failed: $stdout$stderr\n" if $status;
            push @roundtrip_objects, $roundtrip;
        }
    }
    for my $mutation (@{$expect->{object_mutations} // []}) {
        my $bytes = read_file($objects[0]);
        if ($mutation eq 'truncate') { $bytes = substr($bytes, 0, int(length($bytes) / 2)); }
        elsif ($mutation eq 'magic') { substr($bytes, 0, 1) = 'x'; }
        elsif ($mutation eq 'version') { substr($bytes, 4, 1) = "\xff"; }
        elsif ($mutation eq 'header-only') { $bytes = substr($bytes, 0, 8); }
        else { die "$test: unknown public-object mutation $mutation\n"; }
        my $bad = "$directory/$mutation-$level.o";
        write_file($bad, $bytes);
        ($status, $stdout, $stderr) = command("reject-$mutation-$level", $source_compiler,
            '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
            @exports, $bad, '-o', "$directory/rejected.wasm");
        die "$test: malformed object accepted or rejected without a diagnostic: $stdout$stderr\n"
            if !$status || $status >= 128 || "$stdout$stderr" !~ /ERROR:/;
    }
    if ($expect->{archive_units}) {
        @objects = archive_inputs("$directory/library-$level.a", @objects);
        @roundtrip_objects = archive_inputs("$directory/roundtrip-library-$level.a", @roundtrip_objects);
    }
    my $linked = "$directory/linked-$level.wasm";
    for my $pass (1, 2) {
        my $output = "$linked.$pass";
        ($status, $stdout, $stderr) = command("link-$level-$pass", $source_compiler,
            '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
            @target_flags, @{$expect->{link_flags} // []}, @exports, @objects, @runtime_libraries, '-o', $output);
        die "$test: object link failed: $stdout$stderr\n" if $status;
    }
    die "$test: identical object inputs produced different module bytes\n"
        if read_file("$linked.1") ne read_file("$linked.2");
    ($status, $stdout, $stderr) = command("linked-validate-$level", $validator, 'validate', "$linked.1");
    die "$test: invalid linked module: $stdout$stderr\n" if $status;
    push @modules, "$linked.1";
    if (@roundtrip_objects) {
        my $output = "$directory/roundtrip-linked-$level.wasm";
        ($status, $stdout, $stderr) = command("roundtrip-link-$level", $source_compiler,
            '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
            @target_flags, @{$expect->{link_flags} // []}, @exports, @roundtrip_objects, @runtime_libraries, '-o', $output);
        die "$test: round-trip object link failed: $stdout$stderr\n" if $status;
        ($status, $stdout, $stderr) = command("roundtrip-link-validate-$level", $validator, 'validate', $output);
        die "$test: invalid round-trip linked module: $stdout$stderr\n" if $status;
        push @modules, $output;
    }
    if ($expect->{llvm_interop}) {
        die "$test: LLVM interoperability needs C++ source units\n" unless $expect->{source};
        my $clang = $ENV{CPPGM_WASM_CLANG} // 'wasm-ref-clang++';
        my $lld = $ENV{CPPGM_WASM_LD} // 'wasm-ref-ld';
        my @reference_objects;
        for my $index (0 .. $#inputs) {
            my $object = "$directory/llvm-$index-$level.o";
            ($status, $stdout, $stderr) = command("llvm-object-$index-$level", $clang,
                '--target=' . ($expect->{target} // 'wasm32-unknown-unknown'), '-std=c++11', '-fno-exceptions',
                @{$expect->{reference_flags} // []}, "-O$level", '-x', 'c++', '-c', $inputs[$index], '-o', $object);
            die "$test: pinned LLVM compile failed: $stdout$stderr\n" if $status;
            push @reference_objects, $object;
        }
        for my $producer (0 .. $#inputs) {
            my @mixed = @objects;
            $mixed[$producer] = $reference_objects[$producer];
            for my $linker ('cppgm', 'llvm') {
                my $output = "$directory/mixed-$producer-$linker-$level.wasm";
                my @tool = $linker eq 'cppgm'
                    ? ($source_compiler, '--target', ($expect->{target} // 'wasm32-unknown-unknown'), @target_flags)
                    : $expect->{wasi} ? ($clang, '--target=wasm32-wasip1') : ($lld, '--no-entry');
                ($status, $stdout, $stderr) = command("mixed-link-$producer-$linker-$level",
                    @tool, @exports, @mixed, @runtime_libraries, '-o', $output);
                die "$test: mixed-object $linker link failed: $stdout$stderr\n" if $status;
                ($status, $stdout, $stderr) = command("mixed-validate-$producer-$linker-$level",
                    $validator, 'validate', $output);
                die "$test: invalid mixed-object module: $stdout$stderr\n" if $status;
                push @modules, $output;
            }
        }
    }
    for my $run_module (@modules) {
        if ($expect->{abi}) {
            my $actual = export_signatures($run_module);
            for my $name (sort keys %{$expect->{abi}}) {
                die "$test: missing ABI export $name\n" unless $actual->{$name};
                for my $part ('params', 'results') {
                    my $want = join(',', @{$expect->{abi}{$name}{$part}});
                    my $got = join(',', @{$actual->{$name}{$part}});
                    die "$test: $name ABI $part expected [$want], got [$got]\n" if $want ne $got;
                }
            }
        }
        for my $run (@{$expect->{commands} // []}) {
            ($status, $stdout, $stderr) = command("command-$level", $runtime, 'run',
                $run_module, @{$run->{args} // []});
            die "$test: WASI command exit expected $run->{exit}, got $status: $stdout$stderr\n"
                if $status != $run->{exit};
            die "$test: WASI command output expected '$run->{stdout}', got '$stdout'\n"
                if $stdout ne $run->{stdout};
        }
        for my $call (@{$expect->{calls} // []}) {
            ($status, $stdout, $stderr) = command("run-$level-$call->{export}", $runtime, 'run',
                '--invoke', $call->{export}, $run_module, @{$call->{args} // []});
            $stdout =~ s/\s+\z//;
            die "$test: $call->{export} failed ($status): $stdout$stderr\n" if $status;
            die "$test: $call->{export} expected $call->{result}, got '$stdout' from $run_module\n"
                if $stdout ne "$call->{result}";
        }
    }
}
print "Wasm behavior: PASS\n";
