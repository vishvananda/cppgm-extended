#!/usr/bin/perl

use strict;
use warnings;
use utf8 ();
use File::Basename qw(dirname);
use File::Spec;
use File::Temp qw(tempdir);
use FindBin;
use JSON::PP qw(decode_json);
use lib $FindBin::Bin;
use CppgmBatchWorker qw(collect_tests run_command_capture);
use CppgmWasmModule qw(export_signatures export_tag_signatures sequence_module redirect_wasi_exit);
use CppgmWasmFixture qw(mutate_object);
use CppgmWasmQuality qw(check_quality);
use CppgmWasmGlobals qw(initialize_globals metadata_first group_object);

sub read_file {
    my ($path) = @_;
    open(my $file, '<', $path) or die "Cannot read $path: $!\n";
    local $/;
    my $text = <$file>;
    return defined($text) ? $text : '';
}

sub write_file {
    my ($path, $text) = @_;
    utf8::encode($text) if utf8::is_utf8($text);
    open(my $file, '>:raw', $path) or die "Cannot write $path: $!\n";
    print {$file} $text;
    close($file) or die "Cannot close $path: $!\n";
}

sub expectation {
    my ($test) = @_;
    $test =~ s/\.t$/.wasm-expect/;
    return $test;
}

# JSON names are Unicode strings; the module format, argv and file contents
# use UTF-8 bytes. Normalize both values and interface-name keys once.
sub fixture_strings {
    my ($value) = @_;
    return [map { fixture_strings($_) } @$value] if ref($value) eq 'ARRAY';
    if (ref($value) eq 'HASH') {
        my %out;
        for my $name (keys %$value) {
            my $key = $name;
            utf8::encode($key) if utf8::is_utf8($key);
            $out{$key} = fixture_strings($value->{$name});
        }
        return \%out;
    }
    utf8::encode($value) if !ref($value) && utf8::is_utf8($value);
    return $value;
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
my $expect = fixture_strings(decode_json(read_file(expectation($test))));
my $directory = tempdir('cppgm-wasm-XXXXXX', TMPDIR => 1, CLEANUP => 1);
my $validator = $ENV{CPPGM_WASM_TOOLS} // 'wasm-tools';
my $runtime = $ENV{CPPGM_WASMTIME} // 'wasmtime';
my $source_compiler = $ENV{CPPGM_WASM_CPPGM} // File::Spec->catfile(dirname($compiler),
    $compiler =~ /-ref\z/ ? 'cppgm++-ref' : 'cppgm++');
my $command_sequence = 0;
my @units = map { File::Spec->catfile(dirname($test), $_) } @{$expect->{units} // []};

if ($expect->{reference_limitations} || ($expect->{llvm_interop} && $expect->{interop_linkers} &&
    !grep { $_ eq 'llvm' } @{$expect->{interop_linkers}})) {
    my $path = File::Spec->catfile($FindBin::Bin, '..', 'doc', 'wasm', 'reference-limitations.json');
    my $limits = decode_json(read_file($path));
    my %limits = map { $_->{id} => $_ } @{$limits->{limitations}};
    my $relative = File::Spec->abs2rel(File::Spec->rel2abs($test), File::Spec->catdir($FindBin::Bin, '..'));
    my @reasons = @{$expect->{reference_limitations} // []};
    die "$test: omitted reference linker needs a documented limitation\n" unless @reasons;
    for my $reason (@reasons) {
        die "$test: reference limitation does not cover this fixture\n"
            unless $limits{$reason} && grep { $_ eq $relative } @{$limits{$reason}{fixtures}};
    }
}

sub command {
    my ($name, @args) = @_;
    $name .= '-' . ++$command_sequence;
    my $status = run_command_capture(cmd => \@args,
        stdout => "$directory/$name.stdout", stderr => "$directory/$name.stderr", timeout => 30,
        internal_error => $args[0] eq $runtime ? undef : 70,
        supervisor => $compiler, limits => {cpu_seconds => 30, rss_bytes => 2147483648, file_bytes => 67108864});
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

my @preloads;
for my $name (sort keys %{$expect->{hosts} // {}}) {
    die "$test: host fixture module name cannot contain '='\n" if $name =~ /=/;
    my $source = File::Spec->catfile(dirname($test), $expect->{hosts}{$name});
    my $module = "$directory/host-" . ++$command_sequence . '.wasm';
    my ($status, $stdout, $stderr) = command('host-module', $validator, 'parse', $source, '-o', $module);
    die "$test: host module failed: $stdout$stderr\n" if $status;
    ($status, $stdout, $stderr) = command('host-validate', $validator, 'validate', $module);
    die "$test: invalid host module: $stdout$stderr\n" if $status;
    push @preloads, '--preload', "$name=$module";
}

sub standalone_optimizer {
    my ($level, $lowir) = @_;
    my $suffix = $source_compiler =~ /(-ref|-self)\z/ ? $1 : '';
    my $optimizer = $ENV{CPPGM_WASM_LOWIROPT} // File::Spec->catfile(dirname($source_compiler), "lowiropt$suffix");
    my $optimized = "$directory/optimized-$level.lowir";
    my ($status, $stdout, $stderr) = command('standalone-optimize', $optimizer,
        "-O$level", '--validate', '--verify-passes', $lowir, '-o', $optimized);
    die "$test: standalone optimizer failed: $stdout$stderr\n" if $status;
    my $object = "$directory/optimized-$level.o";
    ($status, $stdout, $stderr) = command('optimized-object', $source_compiler,
        '--target', ($expect->{target} // 'wasm32-unknown-unknown'), '-O0', '-c', $optimized, '-o', $object);
    die "$test: optimized object failed: $stdout$stderr\n" if $status;
    ($status, $stdout, $stderr) = command('optimized-object-validate', $validator, 'validate', $object);
    die "$test: optimized object is invalid: $stdout$stderr\n" if $status;
    if ($expect->{allowed_function_imports}) {
        my %allowed = map { $_ => 1 } @{$expect->{allowed_function_imports}};
        my (undef, undef, $interface) = CppgmWasmModule::export_interfaces($object);
        for my $import (@{$interface->{function_imports}}) {
            die "$test: optimizer introduced unavailable function $import->[1]\n" unless $allowed{$import->[1]};
        }
    }
    return $object;
}

sub reference_link_rejection {
    my ($level, $inputs, $exports) = @_;
    die "$test: reference rejection needs one freestanding C++ source\n"
        unless $expect->{source} && !@units && !$expect->{wasi};
    my $clang = $ENV{CPPGM_WASM_CLANG} // 'wasm-ref-clang++';
    my $lld = $ENV{CPPGM_WASM_LD} // 'wasm-ref-ld';
    my $object = "$directory/reference-reject-$level.o";
    my ($status, $stdout, $stderr) = command('reference-reject-source', $clang,
        '--target=' . ($expect->{target} // 'wasm32-unknown-unknown'), '-std=c++11',
        @{$expect->{reference_flags} // []}, "-O$level", '-x', 'c++', '-c', $test, '-o', $object);
    die "$test: reference rejection source did not compile: $stdout$stderr\n" if $status;
    ($status, $stdout, $stderr) = command('reference-reject-object', $validator, 'validate', $object);
    die "$test: invalid reference rejection source object: $stdout$stderr\n" if $status;
    ($status, $stdout, $stderr) = command('reference-reject-link', $lld, '--no-entry',
        @$exports, $object, @$inputs, '-o', "$directory/reference-rejected.wasm");
    die "$test: reference linker accepted incompatible inputs or failed without a diagnostic: $stdout$stderr\n"
        if !$status || $status == 124 || $status >= 128 || "$stdout$stderr" !~ /error/i;
}

my @runtime_libraries;
for my $library (@{$expect->{libraries} // []}) {
    my $clang = $ENV{CPPGM_WASM_CLANG} // 'wasm-ref-clang++';
    my ($status, $stdout, $stderr) = command('library-path', $clang, '--target=wasm32-wasip1',
        @{$expect->{reference_flags} // []}, "-print-file-name=lib$library.a");
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

my @assembly_objects;
for my $index (0 .. $#{$expect->{reference_assembly} // []}) {
    my $input = File::Spec->catfile(dirname($test), $expect->{reference_assembly}[$index]);
    my $output = "$directory/reference-$index.o";
    my $clang = $ENV{CPPGM_WASM_CLANG} // 'wasm-ref-clang++';
    my ($status, $stdout, $stderr) = command('reference-assembly', $clang,
        '--target=' . ($expect->{target} // 'wasm32-unknown-unknown'),
        @{$expect->{reference_flags} // []}, '-x', 'assembler', '-c', $input, '-o', $output);
    die "$test: reference object assembly failed: $stdout$stderr\n" if $status;
    if ($index == 0 && $expect->{reference_initializers}) {
        write_file($output, initialize_globals(read_file($output), $expect->{reference_initializers}));
    }
    if ($expect->{reference_groups}) {
        write_file($output, group_object(read_file($output), $expect->{reference_groups}));
    }
    if ($index == 0 && $expect->{reference_metadata_first}) {
        write_file($output, metadata_first(read_file($output)));
    }
    ($status, $stdout, $stderr) = command('reference-object-validate', $validator, 'validate', $output);
    die "$test: invalid supplied reference object: $stdout$stderr\n" if $status;
    push @assembly_objects, $output;
}

for my $level (@{$expect->{levels} // [0, 1, 2, 3]}) {
    my ($status, $stdout, $stderr);
    my @provided_objects = @assembly_objects;
    for my $index (0 .. $#{$expect->{reference_sources} // []}) {
        my $input = File::Spec->catfile(dirname($test), $expect->{reference_sources}[$index]);
        my $output = "$directory/reference-source-$index-$level.o";
        my $clang = $ENV{CPPGM_WASM_CLANG} // 'wasm-ref-clang++';
        ($status, $stdout, $stderr) = command('reference-source', $clang,
            '--target=' . ($expect->{target} // 'wasm32-unknown-unknown'),
            '-std=c++11', @{$expect->{reference_flags} // []}, "-O$level", '-x', 'c++', '-c', $input, '-o', $output);
        die "$test: reference source compilation failed: $stdout$stderr\n" if $status;
        ($status, $stdout, $stderr) = command('reference-source-validate', $validator, 'validate', $output);
        die "$test: invalid reference source object: $stdout$stderr\n" if $status;
        push @provided_objects, $output;
    }
    my @supplied_inputs = (@provided_objects, @runtime_libraries);
    my @modules;
    my %initialize;
    my $module = "$directory/program-$level.wasm";
    my @sequence_exports = map { $_->{export} } map { @$_ } @{$expect->{sequences} // []};
    my @exports = map { ('--export', $_) } @{$expect->{link_exports} //
        [(map { $_->{export} } @{$expect->{calls} // []}), @{$expect->{exports} // []}, @sequence_exports]};
    if (!$expect->{archive_units}) {
    my $tool = $expect->{source} ? $source_compiler : $compiler;
    ($status, $stdout, $stderr) = command("compile-$level", $tool, "-O$level",
        '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
        @target_flags, @{$expect->{flags} // []}, @{$expect->{link_flags} // []}, @exports, '-o', $module, $test, @units, @supplied_inputs);
    if ($expect->{reject} || $expect->{reject_link}) {
        die "$test: invalid input accepted at O$level\n" if $status == 0;
        die "$test: rejection was not a compiler diagnostic: $stdout$stderr\n"
            if "$stdout$stderr" !~ /ERROR:/ || $status >= 128;
        reference_link_rejection($level, \@supplied_inputs, \@exports) if $expect->{reject_link};
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
            "-O$level", @target_flags, @{$expect->{link_flags} // []}, @exports, '-o', $roundtrip, $lowir, @supplied_inputs);
        die "$test: LowIR backend failed: $stdout$stderr\n" if $status;
        ($status, $stdout, $stderr) = command("roundtrip-validate-$level", $validator, 'validate', $roundtrip);
        die "$test: invalid round-trip module: $stdout$stderr\n" if $status;
        push @modules, $roundtrip;
        if ($expect->{standalone_optimizer}) {
            my $object = standalone_optimizer($level, $lowir);
            my $output = "$directory/standalone-$level.wasm";
            ($status, $stdout, $stderr) = command('standalone-link', $source_compiler,
                '--target', ($expect->{target} // 'wasm32-unknown-unknown'), @target_flags,
                @{$expect->{link_flags} // []}, @exports, $object, @supplied_inputs, '-o', $output);
            die "$test: standalone optimized link failed: $stdout$stderr\n" if $status;
            ($status, $stdout, $stderr) = command('standalone-validate', $validator, 'validate', $output);
            die "$test: standalone optimized module is invalid: $stdout$stderr\n" if $status;
            push @modules, $output;
        }
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
            @target_flags, @{$expect->{link_flags} // []}, @exports, @objects, @supplied_inputs, '-o', $output);
        die "$test: object link failed: $stdout$stderr\n" if $status;
    }
    die "$test: identical object inputs produced different module bytes\n"
        if read_file("$linked.1") ne read_file("$linked.2");
    ($status, $stdout, $stderr) = command("linked-validate-$level", $validator, 'validate', "$linked.1");
    die "$test: invalid linked module: $stdout$stderr\n" if $status;
    push @modules, "$linked.1";
    for my $entry (@{$expect->{reference_object_mutations} // []}) {
        die "$test: reference mutation needs a supplied assembly object\n" if !@provided_objects;
        my $unit = ref($entry) ? $entry->{unit} : 0;
        my $mutation = ref($entry) ? $entry->{kind} : $entry;
        die "$test: invalid reference mutation unit\n" if $unit < 0 || $unit >= @provided_objects;
        my $bad = "$directory/bad-reference-$mutation.o";
        write_file($bad, mutate_object(read_file($provided_objects[$unit]), $mutation));
        my @other = @provided_objects[grep { $_ != $unit } 0 .. $#provided_objects];
        ($status, $stdout, $stderr) = command('reject-reference', $source_compiler,
            '--target', ($expect->{target} // 'wasm32-unknown-unknown'), @target_flags,
            @{$expect->{link_flags} // []}, @exports, @objects, $bad, @other,
            @runtime_libraries, '-o', "$directory/rejected-reference.wasm");
        die "$test: malformed $mutation reference accepted or rejected without a diagnostic: $stdout$stderr\n"
            if !$status || $status == 124 || $status >= 128 || "$stdout$stderr" !~ /ERROR:/;
        ($status, $stdout, $stderr) = command('reference-mutation-object-validate', $validator, 'validate', $bad);
        if ($status) {
            die "$test: reference validator failed without a diagnostic: $stdout$stderr\n"
                if $status == 124 || $status >= 128 || "$stdout$stderr" !~ /error/i;
            next;
        }
        my $oracle = "$directory/oracle-$mutation.wasm";
        my $lld = $ENV{CPPGM_WASM_LD} // 'wasm-ref-ld';
        ($status, $stdout, $stderr) = command('reference-mutation-link', $lld,
            '--no-entry', @exports, @objects, $bad, @other, @runtime_libraries, '-o', $oracle);
        die "$test: reference linker failed without a diagnostic: $stdout$stderr\n"
            if $status && ($status == 124 || $status >= 128 || "$stdout$stderr" !~ /error/i);
        if (!$status) {
            ($status, $stdout, $stderr) = command('reference-mutation-validate', $validator, 'validate', $oracle);
            die "$test: $mutation does not violate the independent reference contract\n" if !$status;
            die "$test: reference validator failed without a diagnostic: $stdout$stderr\n"
                if $status == 124 || $status >= 128 || "$stdout$stderr" !~ /error/i;
        }
    }
    if (@roundtrip_objects) {
        my $output = "$directory/roundtrip-linked-$level.wasm";
        ($status, $stdout, $stderr) = command("roundtrip-link-$level", $source_compiler,
            '--target', ($expect->{target} // 'wasm32-unknown-unknown'),
            @target_flags, @{$expect->{link_flags} // []}, @exports, @roundtrip_objects, @supplied_inputs, '-o', $output);
        die "$test: round-trip object link failed: $stdout$stderr\n" if $status;
        ($status, $stdout, $stderr) = command("roundtrip-link-validate-$level", $validator, 'validate', $output);
        die "$test: invalid round-trip linked module: $stdout$stderr\n" if $status;
        push @modules, $output;
    }
    if ($expect->{llvm_link}) {
        my $lld = $ENV{CPPGM_WASM_LD} // 'wasm-ref-ld';
        my $output = "$directory/llvm-link-$level.wasm";
        ($status, $stdout, $stderr) = command("llvm-link-$level", $lld,
            '--no-entry', @{$expect->{reference_link_flags} // []}, @exports, @objects, @supplied_inputs, '-o', $output);
        die "$test: reference link failed: $stdout$stderr\n" if $status;
        ($status, $stdout, $stderr) = command("llvm-link-validate-$level", $validator, 'validate', $output);
        die "$test: invalid reference-linked module: $stdout$stderr\n" if $status;
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
            for my $linker (@{$expect->{interop_linkers} // ['cppgm', 'llvm']}) {
                die "$test: unknown interoperability linker $linker\n" unless $linker eq 'cppgm' || $linker eq 'llvm';
                my $output = "$directory/mixed-$producer-$linker-$level.wasm";
                my @tool = $linker eq 'cppgm'
                    ? ($source_compiler, '--target', ($expect->{target} // 'wasm32-unknown-unknown'), @target_flags, @{$expect->{link_flags} // []})
                    : $expect->{wasi} ? ($clang, '--target=wasm32-wasip1', @{$expect->{link_flags} // []}, @{$expect->{reference_link_flags} // []}) : ($lld, '--no-entry', @{$expect->{reference_link_flags} // []});
                my @link_exports = @exports;
                if ($linker eq 'llvm' && $expect->{reference_initialize}) {
                    # Exporting the constructor entry disables LLD's per-command
                    # wrappers. The reference reactor host calls it once.
                    push @link_exports, '--export', $expect->{reference_initialize};
                    $initialize{$output} = $expect->{reference_initialize};
                }
                @link_exports = map { ('-Xlinker', $_) } @link_exports
                    if $linker eq 'llvm' && $expect->{wasi};
                ($status, $stdout, $stderr) = command("mixed-link-$producer-$linker-$level",
                    @tool, @link_exports, @mixed, @supplied_inputs, '-o', $output);
                die "$test: mixed-object $linker link failed: $stdout$stderr\n" if $status;
                ($status, $stdout, $stderr) = command("mixed-validate-$producer-$linker-$level",
                    $validator, 'validate', $output);
                die "$test: invalid mixed-object module $output: $stdout$stderr\n" if $status;
                push @modules, $output;
            }
        }
    }
    for my $run_module (@modules) {
        if ($expect->{export_names}) {
            my $actual = export_signatures($run_module);
            die "$test: function export names differ in $run_module: expected [" .
                join(', ', sort @{$expect->{export_names}}) . '], got [' . join(', ', sort keys %$actual) . "]\n"
                if scalar(keys %$actual) != scalar(@{$expect->{export_names}}) ||
                   join("\0", sort keys %$actual) ne join("\0", sort @{$expect->{export_names}});
        }
        check_quality($run_module, $expect->{quality}, $level) if $expect->{quality};
        if ($expect->{tag_abi}) {
            my $actual = export_tag_signatures($run_module);
            for my $name (sort keys %{$expect->{tag_abi}}) {
                die "$test: missing exported tag $name\n" unless $actual->{$name};
                for my $part ('params', 'results') {
                    my $want = join(',', @{$expect->{tag_abi}{$name}{$part}});
                    my $got = join(',', @{$actual->{$name}{$part}});
                    die "$test: tag $name ABI $part expected [$want], got [$got]\n" if $want ne $got;
                }
            }
        }
        if ($expect->{abi}) {
            my $actual = export_signatures($run_module);
            for my $name (sort keys %{$expect->{abi}}) {
                die "$test: missing ABI export $name in $run_module\n" unless $actual->{$name};
                for my $part ('params', 'results') {
                    my $want = join(',', @{$expect->{abi}{$name}{$part}});
                    my $got = join(',', @{$actual->{$name}{$part}});
                    die "$test: $name ABI $part expected [$want], got [$got]\n" if $want ne $got;
                }
            }
        }
        for my $run (@{$expect->{commands} // []}) {
            my $command_module = $run_module;
            my @exit_host;
            my @exit_entry;
            if (exists $run->{proc_exit}) {
                my $code = $run->{proc_exit};
                die "$test: invalid proc_exit assertion\n"
                    unless "$code" =~ /\A[0-9]+\z/ && $code <= 0xffffffff && $run->{exit} == 0;
                my $prefix = "$directory/exit-" . ++$command_sequence;
                $command_module = "$prefix.wasm";
                redirect_wasi_exit($run_module, $command_module);
                # Returning from _start must fail too: only the assertion host
                # may terminate successfully with the expected empty output.
                @exit_entry = ('--invoke', sequence_module($command_module, $command_module, [], '_start'));
                write_file("$prefix.wat", '(module (import "wasi_snapshot_preview1" "proc_exit"'
                    . ' (func $exit (param i32))) (memory (export "memory") 1)'
                    . ' (func (export "proc_exit") (param i32)'
                    . " local.get 0 i32.const $code i32.ne call \$exit unreachable))");
                ($status, $stdout, $stderr) = command('exit-host', $validator, 'parse',
                    "$prefix.wat", '-o', "$prefix-host.wasm");
                die "$test: exit assertion host failed: $stdout$stderr\n" if $status;
                @exit_host = ('--preload', "cppgm_test_exit=$prefix-host.wasm");
            }
            my @preopen;
            my $files;
            if ($run->{files} || $run->{outputs}) {
                $files = tempdir('command-files-XXXXXX', DIR => $directory);
                for my $name (keys %{$run->{files} // {}}, keys %{$run->{outputs} // {}}) {
                    die "$test: invalid command fixture filename\n"
                        unless $name =~ /^[a-zA-Z0-9_-][a-zA-Z0-9_.-]*$/;
                }
                write_file("$files/$_", $run->{files}{$_}) for keys %{$run->{files} // {}};
                @preopen = ('--dir', "${files}::.");
            }
            ($status, $stdout, $stderr) = command("command-$level", $runtime, 'run', '-W', 'max-memory-size=268435456',
                @preloads, @exit_host, @exit_entry, @preopen, $command_module, @{$run->{args} // []});
            die "$test: WASI command exit expected $run->{exit}, got $status: $stdout$stderr\n"
                if $status != $run->{exit};
            die "$test: WASI command output expected '$run->{stdout}', got '$stdout'\n"
                if $stdout ne $run->{stdout};
            for my $name (sort keys %{$run->{outputs} // {}}) {
                die "$test: command file $name differs\n"
                    if read_file("$files/$name") ne $run->{outputs}{$name};
            }
        }
        for my $sequence (@{$expect->{sequences} // []}) {
            my $wrapped = "$directory/sequence-" . ++$command_sequence . '.wasm';
            my $entry = sequence_module($run_module, $wrapped, $sequence, $initialize{$run_module});
            ($status, $stdout, $stderr) = command("sequence-validate-$level", $validator, 'validate', $wrapped);
            die "$test: invalid sequence driver: $stdout$stderr\n" if $status;
            ($status, $stdout, $stderr) = command("sequence-run-$level", $runtime, 'run', '-W', 'max-memory-size=268435456', @preloads, '--invoke', $entry, $wrapped);
            $stdout =~ s/\s+\z//;
            if (!$status && $stdout =~ /^-([0-9]+)$/ && $1 <= @$sequence) {
                my $call = $sequence->[$1 - 1];
                die "$test: same-instance assertion $1 ($call->{export}) expected $call->{result} in $run_module\n";
            }
            die "$test: same-instance sequence failed ($status): $stdout$stderr\n"
                if $status || $stdout ne '17';
        }
        for my $call (@{$expect->{calls} // []}) {
            my @fuel = exists($call->{fuel}{$level}) ? ('-W', "fuel=$call->{fuel}{$level}") : ();
            ($status, $stdout, $stderr) = command("run-$level-$call->{export}", $runtime, 'run', '-W', 'max-memory-size=268435456',
                @preloads, @fuel, '--invoke', $call->{export}, $run_module, @{$call->{args} // []});
            $stdout =~ s/\s+\z//;
            if ($call->{trap}) {
                die "$test: $call->{export} did not produce a Wasm trap: $stdout$stderr\n"
                    if !$status || $status == 124 || $stderr !~ /\bwasm trap\b/;
                next;
            }
            die "$test: $call->{export} failed ($status): $stdout$stderr\n" if $status;
            die "$test: $call->{export} expected $call->{result}, got '$stdout' from $run_module\n"
                if $stdout ne "$call->{result}";
        }
    }
}
print "Wasm behavior: PASS\n";
