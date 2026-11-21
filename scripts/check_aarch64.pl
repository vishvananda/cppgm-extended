#!/usr/bin/perl
use strict;
use warnings;
use File::Basename qw(dirname basename);
use File::Path qw(make_path);
use Digest::SHA qw(sha256_hex);
use File::Compare qw(compare);
use File::Copy qw(copy);
use File::Spec;
use File::Temp qw(tempdir);
use FindBin;
use JSON::PP;
use lib $FindBin::Bin;
use CppgmBatchWorker qw(run_command_capture);
use CppgmArm64MachO qw(check_apple_fixture);
use CppgmBackendFixture qw(backend_tests backend_expectation fixture_units read_bytes check_bounds);

if (@ARGV && $ARGV[0] eq '--list') {
    shift @ARGV;
    print "$_\n" for backend_tests('arm', @ARGV);
    exit 0;
}

@ARGV == 2 or die "usage: check_aarch64.pl <cppgm> <fixture>\n";
my ($compiler, $test) = @ARGV;
my $directory = tempdir('cppgm-aarch64-XXXXXX', TMPDIR => 1, CLEANUP => 1);
my $expect = backend_expectation($test,'arm');
if (($expect->{target} // '') eq 'arm64-apple-macos11') {
    check_apple_fixture($compiler, $test, $directory, $expect);
    print "AArch64 control passed\n";
    exit 0;
}
die "$test: PA39 expects a source fixture with command outcomes\n"
    unless $expect->{source} && ($expect->{reject_link} || @{$expect->{commands} // []});
my @units = fixture_units($test,$expect);
my $suffix = $compiler =~ /(-ref|-self)$/ ? $1 : '';
my $bin = dirname($compiler);
my $opt = "$bin/lowiropt$suffix";
my $backend = "$bin/lowir2native$suffix";
my @target = ('--target=aarch64-linux-gnu');
my $clang = $ENV{CPPGM_AARCH64_CLANG} // 'clang';
my $lld = $ENV{CPPGM_AARCH64_LLD} // 'ld.lld';
my $inspect = $ENV{CPPGM_AARCH64_READOBJ} // 'llvm-readobj';
my $objdump = $ENV{CPPGM_AARCH64_OBJDUMP} // 'llvm-objdump';
my $ar = $ENV{CPPGM_AARCH64_AR} // 'llvm-ar';
my $qemu = $ENV{CPPGM_AARCH64_QEMU} // 'qemu-aarch64';
my $support = "$FindBin::Bin/../pa39/support";
my @assembly;
my @libraries;
if ($expect->{builtins}) {
    my $archive = $ENV{CPPGM_AARCH64_BUILTINS} // '/usr/lib/gcc-cross/aarch64-linux-gnu/14/libgcc.a';
    -r $archive or die "set CPPGM_AARCH64_BUILTINS to the target libgcc.a or compiler-rt builtins archive\n";
    push @libraries, $archive;
}

sub run {
    my ($want, @args) = @_;
    unlink "$directory/out", "$directory/err";
    my $status = run_command_capture(cmd => \@args, stdout => "$directory/out", stderr => "$directory/err",
        timeout => 30, supervisor => $compiler,
        limits => {cpu_seconds => 30, rss_bytes => 2147483648, file_bytes => 67108864});
    die "@args: expected $want, got $status\n" . read_bytes("$directory/out") . read_bytes("$directory/err")
        if $status != $want;
    return read_bytes("$directory/out");
}

my $bundle;
my ($level_name, $producers, @manifest);
if ($ENV{CPPGM_AARCH64_ARTIFACTS}) {
    $bundle = File::Spec->catdir($ENV{CPPGM_AARCH64_ARTIFACTS}, basename(dirname($test)), basename($test, '.t'));
    make_path($bundle);
}
sub execute_image {
    my ($linker) = @_;
    for my $command (@{$expect->{commands}}) {
        my @args = @{$command->{args} // []};
        my $stdout = run($command->{exit}, $qemu, '-cpu', 'cortex-a53', "$directory/program", @args);
        $stdout eq ($command->{stdout} // '') or die "$test: guest stdout differs\n";
        if ($bundle) {
            my $digest = sha256_hex(read_bytes("$directory/program"));
            my $guest = "$bundle/$digest";
            copy("$directory/program", $guest) or die "copy $guest: $!" unless -f $guest;
            push @manifest, join("\t", $digest, $command->{exit}, $level_name, $producers, $linker,
                JSON::PP->new->ascii->canonical->encode($command)) . "\n";
        }
    }
}

sub link_external {
    run(0, $lld, '-m', 'aarch64elf', '-static', '-e', '_start', "$directory/start.o", @_, @assembly, @libraries, '-o', "$directory/program");
    execute_image('lld');
}

sub link_course {
    run(0, $compiler, @target, @_, @assembly, @libraries, '-o', "$directory/program");
    execute_image('course');
}

sub link_hosted {
    my $sysroot = $ENV{CPPGM_AARCH64_SDK_ROOT} // '/';
    my $c = "$sysroot/usr/aarch64-linux-gnu/lib";
    my $gcc = "$sysroot/usr/lib/gcc-cross/aarch64-linux-gnu/14";
    -r "$gcc/libstdc++.a" && -r "$c/crt1.o" or die "missing AArch64 static C++ runtime in $sysroot\n";
    run(0, $lld, "--sysroot=$sysroot", '-m', 'aarch64elf', '-static', '--eh-frame-hdr', '-e', '_start',
        "$c/crt1.o", "$c/crti.o", "$gcc/crtbeginT.o", @_, "-L$gcc", "-L$c", '--start-group',
        '-lstdc++', '-lm', '-lgcc_eh', '-lc', '-lgcc', '--end-group', "$gcc/crtend.o", "$c/crtn.o", '-o', "$directory/program");
    execute_image('lld-hosted');
}

run(0, $clang, @target, '-march=armv8-a', '-c', "$support/start.S", '-o', "$directory/start.o");
for my $asm (@{$expect->{reference_assembly} // []}) {
    push @assembly, "$directory/reference-asm-".scalar(@assembly).'.o';
    run(0, $clang, @target, @{$expect->{reference_flags} // []}, '-c',
        File::Spec->catfile(dirname($test), $asm), '-o', $assembly[-1]);
}
for my $level (@{$expect->{levels} // [0..3]}) {
    $level_name = "O$level";
    $producers = 'clang';
    my @flags = (@target, "-O$level", @{$expect->{flags} // []});
    my @reference = ($clang, '-fno-pic', @flags, '-march=armv8-a', '-mno-outline-atomics', '-x', 'c++');
    my $object = "$directory/unit.o";
    my $other = "$directory/reference.o";
    run(0, @reference, '-c', $test, '-o', $other);
    run(0, $compiler, @flags, '-x', 'c++', '-c', $test, '-o', $object);
    if ($expect->{reject_link}) {
        for my $cmd ([$compiler, @target, $object], [$lld, '-m', 'aarch64elf', '-static', '-e', 'main', $other]) {
            run(1, @$cmd, @assembly, '-o', "$directory/rejected");
            read_bytes("$directory/err") =~ /\S/ or die "$test: link failed without a diagnostic\n";
        }
        next;
    }
    my $header = run(0, $inspect, '--file-headers', '--relocations', $object);
    die "$test: wrong ELF architecture\n" unless $header =~ /Machine: EM_AARCH64\b/;
    die "$test: expected relocation $expect->{relocation}\n"
        if $expect->{relocation} && index($header, $expect->{relocation}) < 0;
    if ($expect->{shared_link}) {
        my $pic_object = "$directory/pic-unit.o";
        run(0, $compiler, @flags, '-fPIC', '-x', 'c++', '-c', $test, '-o', $pic_object);
        run(0, $lld, '-m', 'aarch64elf', '-shared', '--no-undefined', '-z', 'text', $pic_object, '-o', "$directory/libfixture.so");
    }
    my $quality = $expect->{quality};
    if ($quality && grep { $_ eq $level } @{$quality->{levels} // [1, 2, 3]}) {
        # `functions` bounds both compilers' code; `own_functions` bounds only the tested compiler's,
        # for a rule Clang does not follow.
        my %bounded = map { $_ => 1 } keys %{$quality->{functions} // {}}, keys %{$quality->{own_functions} // {}};
        for my $symbol (sort keys %bounded) {
            for my $unit ($other, $object) {
                my $assembly = run(0, $objdump, '-t', '-d', '--no-show-raw-insn', "--disassemble-symbols=$symbol", $unit);
                my ($start, $size) = $assembly =~ /^([0-9a-f]+)\s+[^\n]*?\bF\s+\S+\s+([0-9a-f]+)\s+\Q$symbol\E$/m;
                defined $size or die "$test: no function size for $symbol\n";
                my $end = hex($start) + hex($size);
                my @ins;
                my $back_jumps = 0;
                # llvm-objdump can print the next function's alignment padding.
                while ($assembly =~ /^\s*([0-9a-f]+):\s+(\S+[^\n]*)$/mg) {
                    my ($address, $text) = (hex($1), $2);
                    next if $address < hex($start) || $address >= $end;
                    push @ins, $text;
                    # An unconditional jump to an earlier instruction.
                    $back_jumps++ if $text =~ /^b\s+(?:0x)?([0-9a-f]+)\b/ && hex($1) < $address;
                }
                @ins or die "$test: no instructions for $symbol\n";
                my @memory = grep { /^(?:ld|st)/ } @ins;
                my %counts = (instructions => scalar(@ins), code_bytes => 4 * @ins, memory_ops => scalar(@memory),
                    stack_ops => scalar(grep { /\[sp\b/ } @memory), calls => scalar(grep { /^bl(?:r)?\b/ } @ins),
                    pair_ops => scalar(grep { /^(?:ldp|stp)\b/ } @memory),
                    sp_uses => scalar(grep { /\bsp\b/ } @ins), back_jumps => $back_jumps,
                    flag_values => scalar(grep { /^csetm?\b/ } @ins), jumps => scalar(grep { /^b\s/ } @ins),
                    conditional_branches => scalar(grep { /^(?:b\.\w+|cbn?z|tbn?z)\b/ } @ins),
                    divides => scalar(grep { /^[su]div\b/ } @ins),
                    register_moves => scalar(grep { /^mov\s+[xw]\d+, [xw]\d+$/ } @ins));
                check_bounds(\%counts, $quality->{functions}{$symbol} // {}, "$test: O$level $symbol", 'AArch64');
                check_bounds(\%counts, $quality->{own_functions}{$symbol} // {}, "$test: O$level $symbol", 'AArch64')
                    if $unit eq $object;
            }
        }
    }
    for my $optimized (0,1) {
        my @emit = $optimized ? ("-O$level", '-S', '-emit-lowir') :
            ('-O0', '--emit-lowir', '-g0', ($level ? '-D__OPTIMIZE__=1' : ()), ($level eq 'z' ? '-D__OPTIMIZE_SIZE__=1' : ()));
        run(0, $compiler, @target, @emit, @{$expect->{flags} // []}, '-x', 'c++', $test, '-o', "$directory/unit.lowir");
        run(0, $opt, '-O0', "$directory/unit.lowir", '-o', "$directory/roundtrip.lowir");
        run(0, $compiler, @flags, ($optimized ? '-fno-lowir-passes' : ()), '-c', "$directory/roundtrip.lowir", '-o', "$directory/roundtrip.o");
        copy("$directory/roundtrip.lowir", "$directory/raw.lowir") or die "$!" unless $optimized;
        die "$test: O$level LowIR object differs (optimized=$optimized)\n" if compare($object, "$directory/roundtrip.o");
    }
    if ($expect->{source_link_flags}) {
        my @group = ($compiler, @flags, @{$expect->{source_link_flags}}, '-x', 'c++', $test, @units);
        $producers = 'cppgm-source-lto';
        run(0, @group, '-o', "$directory/program");
        execute_image('course');
        run(0, @group, '-c', '-o', "$directory/combined.o");
        link_external("$directory/combined.o");
    }
    if (@units) {
        my (@clang_peers, @cppgm_peers);
        for my $i (0 .. $#units) {
            push @clang_peers, "$directory/clang-peer-$i.o";
            push @cppgm_peers, "$directory/cppgm-peer-$i.o";
            run(0, @reference, '-c', $units[$i], '-o', $clang_peers[-1]);
            run(0, $compiler, @flags, '-x', 'c++', '-c', $units[$i], '-o', $cppgm_peers[-1]);
        }
        if ($expect->{archive_units}) {
            for my $pair (['witness', \@clang_peers], ['cppgm', \@cppgm_peers]) {
                my ($name, $members) = @$pair;
                my $archive = "$directory/lib$name.a";
                unlink $archive;
                run(0, $ar, 'rcs', $archive, @$members);
                @$members = ($archive);
            }
        }
        my $link = $expect->{hosted} ? \&link_hosted : \&link_external;
        $link->($other, @clang_peers);
        $producers = 'cppgm';
        $link->($object, @cppgm_peers);
        if ($expect->{llvm_interop}) {
            for my $pair ([$object, @clang_peers], [$other, @cppgm_peers]) {
                $producers = $pair->[0] eq $object ? 'cppgm+clang' : 'clang+cppgm';
                $link->(@$pair);
                link_course(@$pair) unless $expect->{hosted};
            }
        }
        if ($expect->{archive_units}) {
            $producers = 'cppgm+clang-archive';
            link_course($object, "-L$directory", '-lwitness');
            run(0, $backend, @target, "-O$level", '--dump-machine-ir', "$directory/unit.mir",
                "$directory/raw.lowir", "$directory/libwitness.a", '-o', "$directory/program");
            execute_image('lowir2native');
            die "$test: missing selected Arm instructions\n" unless
                read_bytes("$directory/unit.mir") =~ /^machine_ir aarch64 1\nfunction [^\n]+\n  0 \w+ rd=/;
        }
    } else {
        link_external($other);
        $producers = 'cppgm';
        link_external($object);
        link_course($object);
        run(0, $backend, @flags, "$directory/raw.lowir", '-o', "$directory/program");
        execute_image('lowir2native');
    }
}
if ($bundle && @manifest) {
    open(my $file, '>', "$bundle/executions.tsv.tmp") or die "manifest: $!";
    print {$file} @manifest or die "manifest: $!";
    close $file or die "manifest: $!";
    rename "$bundle/executions.tsv.tmp", "$bundle/executions.tsv" or die "manifest: $!";
}
print "AArch64 control passed\n";
