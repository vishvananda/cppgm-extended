#!/usr/bin/perl

use strict;
use warnings;
use Cwd qw(abs_path);
use Digest::SHA qw(sha256_hex);
use File::Copy qw(copy);
use File::Path qw(make_path);
use File::Spec;
use FindBin;
use Getopt::Long qw(GetOptions);
use JSON::PP;
use POSIX qw(uname);
use Text::ParseWords qw(shellwords);
use lib $FindBin::Bin;
use CppgmBatchWorker qw(run_command_capture);
use CppgmWasmBenchmark qw(oracle module_sizes summarize median);

my $root = abs_path("$FindBin::Bin/..");
my $repo_root = $root;
my $compiler = "$root/dev/cppgm++";
my $tools = "$root/obj/wasm-tools";
my $output = "$root/obj/wasm-benchmarks/" . time() . "-$$";
my $host_cxx = $ENV{CPPGM_HOST_CXX} // 'g++';
my ($blocks, $aa_blocks, $seconds, $level) = (3, 2, .15, 3);
GetOptions('compiler=s' => \$compiler, 'tools=s' => \$tools, 'output=s' => \$output,
    'repo-root=s' => \$repo_root,
    'host-cxx=s' => \$host_cxx, 'blocks=i' => \$blocks, 'aa-blocks=i' => \$aa_blocks,
    'seconds=f' => \$seconds, 'level=i' => \$level) && !@ARGV
    or die "usage: wasm_benchmarks.pl [--compiler path] [--tools prefix] [--output directory] [--level 0..3]\n";
die "Positive blocks, AA blocks and seconds, and level 0..3 required\n"
    unless $blocks > 0 && $aa_blocks > 0 && $seconds > 0 && $level >= 0 && $level <= 3;
$compiler = abs_path($compiler) // die "Missing compiler\n";
$repo_root = abs_path($repo_root) // die "Missing measured source checkout\n";
$tools = abs_path($tools) // die "Missing pinned tools; run install_wasm_tools.pl --benchmark-engine\n";
$output = File::Spec->rel2abs($output);
die "Use a fresh output directory: $output\n" if -e "$output/results.json";
make_path("$output/raw");
my $json = JSON::PP->new->canonical;
my @commands;
my $command_id = 0;

sub read_file {
    open(my $f, '<:raw', $_[0]) or die "Cannot read $_[0]: $!\n";
    local $/;
    return <$f> // '';
}
sub write_file {
    open(my $f, '>:raw', $_[0]) or die "Cannot write $_[0]: $!\n";
    print {$f} $_[1];
    close($f) or die "Cannot close $_[0]: $!\n";
}
sub save_json { write_file($_[0], $json->pretty->encode($_[1])); }
sub capture {
    my ($cmd, $stdin) = @_;
    my $prefix = sprintf('%s/raw/%04d', $output, ++$command_id);
    my ($status, %outcome);
    eval { $status = run_command_capture(cmd => $cmd, stdout => "$prefix.stdout", stderr => "$prefix.stderr",
        timeout => 120, supervisor => $compiler, outcome => \%outcome, internal_error => 70,
        limits => {cpu_seconds => 120, rss_bytes => 4294967296, file_bytes => 268435456},
        defined($stdin) ? (stdin => $stdin) : ()); };
    my $failure = $@;
    push @commands, {command => $cmd, exit => $status, stdin => $stdin,
        outcome => \%outcome, failure => $failure, stdout => "$prefix.stdout", stderr => "$prefix.stderr"};
    save_json("$output/commands.json", \@commands);
    die $failure if $failure;
    die "Command failed ($status): @$cmd\n" . read_file("$prefix.stderr") . read_file("$prefix.stdout") if $status;
    return read_file("$prefix.stdout");
}
sub digest { return sha256_hex(read_file($_[0])); }

my @affinity;
my @pin;
if ($^O eq 'linux') {
    my $status = read_file('/proc/self/status');
    $status =~ /^Cpus_allowed_list:\s*(\d+)/m or die "Cannot determine CPU affinity\n";
    @affinity = (int($1));
    @pin = ('taskset', '-c', $affinity[0]);
}
my $cpu = $^O eq 'darwin' ? capture(['sysctl', '-n', 'machdep.cpu.brand_string']) : read_file('/proc/cpuinfo');
$cpu = $1 if $cpu =~ /^model name\s*:\s*(.+)$/m;
$cpu =~ s/\s+\z//;
my @uname = uname();
my $revision = capture(['git', '-C', $repo_root, 'rev-parse', 'HEAD']);
$revision =~ s/\s+\z//;
my $dirty = length(capture(['git', '-C', $repo_root, 'status', '--porcelain'])) != 0;
my $engine = "$tools/wasmtime-c-api";
die "Missing pinned C API; run install_wasm_tools.pl --benchmark-engine\n" unless -f "$engine/lib/libwasmtime.a";
my $source = "$output/kernels.cpp";
copy("$repo_root/pa37/benchmarks/kernels.cpp", $source) or die "Cannot freeze input: $!\n";
copy("$repo_root/pa37/benchmarks/applications.h", "$output/applications.h") or die "Cannot freeze applications: $!\n";
my $runner = "$output/runner";
capture([shellwords($host_cxx), '-std=c++11', '-O2', '-Wall', '-Wextra', '-Werror',
    "-I$engine/include", "$root/scripts/wasm_benchmark_runner.cpp", "$engine/lib/libwasmtime.a",
    '-ldl', '-lpthread', '-lm', '-o', $runner]);
my $clang = "$tools/bin/wasm-ref-clang++";
my $lld = "$tools/bin/wasm-ref-ld";
my $validator = "$tools/bin/wasm-tools";
my @flags = ('-std=c++11', "-O$level", '-fno-exceptions', '-fno-strict-aliasing');
my $metadata = {revision => $revision, dirty => $dirty ? JSON::PP::true : JSON::PP::false,
    cpu => $cpu, architecture => $uname[4], operating_system => join(' ', @uname[0,2,3]), affinity => \@affinity,
    source_sha256 => sha256_hex(read_file($source) . read_file("$output/applications.h")), compiler_sha256 => digest($compiler), clang_sha256 => digest($clang),
    linker_sha256 => digest($lld), runner_sha256 => digest($runner), engine_sha256 => digest("$engine/lib/libwasmtime.a"),
    tool_manifest_sha256 => digest("$FindBin::Bin/wasm_tools.json"), optimization => "O$level",
    engine => {version => '50.0.1', compiler => 'Cranelift', optimization => 'speed', simd => JSON::PP::true,
        parallel_compilation => JSON::PP::false, cache => JSON::PP::false, fuel => JSON::PP::false,
        guest_memory_bytes => 268435456, guest_table_elements => 100000,
        guest_instances => 16, guest_tables => 16, guest_memories => 16},
    resources => {wall_seconds => 120, cpu_seconds => 120, rss_bytes => 4294967296,
        file_bytes => 268435456, rss_sample_ms => 10},
    clang_version => capture([$clang, '--version']), flags => {cppgm => \@flags, clang => [@flags, '-msimd128']}};
$metadata->{host_cxx_version} = capture([shellwords($host_cxx), '--version']);
save_json("$output/metadata.json", $metadata);
save_json("$output/results.json", {schema => 2, status => 'partial', metadata => $metadata});
my @names = qw(hash filter matrix transform sort graph json lzss);
my @cases = map {my $kind = $_; map {my $seed = $_; map {
    [$kind, $_, $seed, oracle($kind, $_, $seed)]} (1,3)} (17,42,0x80000000,0xffffffff)} (0..$#names);
my @warnings;
my @configurations;

sub batch {
    my ($modules, $requests, $name) = @_;
    my $input = "$output/raw/requests-" . ++$command_id . '.stdin';
    write_file($input, join('', map {join(' ', @$_) . "\n"} @$requests));
    my @rows = map {$json->decode($_)} split /\n/, capture([@pin, $runner, @$modules], $input);
    my @runs = grep {$_->{event} eq 'run'} @rows;
    die "Incomplete engine response\n" unless @runs == @$requests;
    for my $i (0 .. $#runs) {
        die "Engine response changed request\n" unless join(',', @{$requests->[$i]}) eq
            join(',', @{$runs[$i]}{qw(slot kind repeats seed)});
    }
    save_json("$output/raw/$name.json", \@rows);
    return \@runs;
}

for my $simd (0,1) {
    my $configuration = $simd ? 'simd128' : 'scalar';
    my $directory = "$output/$configuration";
    make_path($directory);
    my @objects = map {"$directory/$_.o"} qw(cppgm clang);
    my @modules = map {"$directory/$_.wasm"} qw(cppgm clang);
    my @compile = ([$compiler, '--target=wasm32-unknown-unknown', @flags, "-DCPPGM_BENCH_SIMD=$simd", '-c', $source, '-o', $objects[0]],
        [$clang, '--target=wasm32-unknown-unknown', @flags, '-msimd128', "-DCPPGM_BENCH_SIMD=$simd", '-c', $source, '-o', $objects[1]]);
    my @link = ([$compiler, '--target=wasm32-unknown-unknown', '--export', 'prepare', '--export', 'run', $objects[0], '-o', $modules[0]],
        [$lld, '--no-entry', '--export=prepare', '--export=run', $objects[1], '-o', $modules[1]]);
    my %phases;
    for my $phase ('compile', 'link') {
        my @command = $phase eq 'compile' ? @compile : @link;
        capture($_) for @command; # Warm tool/file caches; not a measurement.
        my @gold = map {digest($_)} $phase eq 'compile' ? @objects : @modules;
        for my $mode ('AA','AB') {
            my @rows;
            for my $block (1 .. ($mode eq 'AA' ? $aa_blocks : $blocks)) {
                for my $slot (0,1,1,0) {
                    my $actual = $mode eq 'AA' ? 0 : $slot;
                    my $row = $json->decode(capture([@pin, $runner, '--time', @{$command[$actual]}]));
                    my $path = $phase eq 'compile' ? $objects[$actual] : $modules[$actual];
                    die "Nondeterministic $phase output\n" unless digest($path) eq $gold[$actual];
                    push @rows, {%$row, slot => $slot, output_sha256 => digest($path)};
                }
            }
            save_json("$directory/$phase-$mode.json", \@rows);
            $phases{$phase}{$mode} = {map {$_ => summarize(\@rows, $_)} qw(seconds cpu_seconds)};
        }
        for my $metric ('seconds', 'cpu_seconds') {
            my $aa = $phases{$phase}{AA}{$metric}{speedup};
            push @warnings, "$configuration/$phase/$metric A/A ratio $aa exceeds 5%" if abs($aa-1) > .05;
        }
    }
    capture([$validator, 'validate', $_]) for @modules;
    my @oracle_requests = map {my $slot = $_; map {[$slot, @$_[0..2]]} @cases} (0,1);
    my $checked = batch(\@modules, \@oracle_requests, "$configuration-oracles");
    for my $i (0 .. $#$checked) {
        die "Independent oracle mismatch in $configuration case $i\n"
            unless $checked->[$i]{result} == $cases[$i % @cases][3];
    }
    for my $kind (0 .. $#names, 0xffffffff) {
        my $name = $kind == 0xffffffff ? 'startup' : $names[$kind];
        my $repeats = $kind == 0xffffffff ? 100 : 16;
        if ($kind != 0xffffffff) {
            for my $round (0 .. 8) {
                my $row = batch(\@modules, [[1,$kind,$repeats,42]], "$configuration-$name-calibrate-$round")->[0];
                last if $row->{seconds} >= $seconds;
                die "Cannot reach timing floor\n" if $round == 8;
                my $factor = int($seconds / $row->{seconds}) + 1;
                $factor = 2 if $factor < 2;
                $factor = 100 if $factor > 100;
                $repeats *= $factor;
                die "Excessive repeat count\n" if $repeats > 100000000;
            }
        }
        my $expected_result;
        for my $mode ('AA','AB') {
            my @requests = map {map {[$_,$kind,$repeats,42]} (0,1,1,0)}
                1 .. ($mode eq 'AA' ? $aa_blocks : $blocks);
            my @selected = $mode eq 'AA' ? ($modules[0],$modules[0]) : @modules;
            my $rows = batch(\@selected, \@requests, "$configuration-$name-$mode");
            $expected_result //= $rows->[0]{result};
            die "Runtime checksums differ in $configuration/$name\n"
                if grep {$_->{result} != $expected_result} @$rows;
            $phases{$name}{$mode} = {seconds => summarize($rows, 'seconds'), repeats => $repeats};
            $phases{$name}{$mode}{cpu_seconds} = summarize($rows, 'cpu_seconds') if $kind != 0xffffffff;
            $phases{$name}{$mode}{setup_seconds} = summarize($rows, 'setup_seconds') if $kind != 0xffffffff;
        }
        my $aa = $phases{$name}{AA}{seconds}{speedup};
        push @warnings, "$configuration/$name A/A ratio $aa exceeds 5%" if abs($aa-1) > .05;
    }
    push @configurations, {id => $configuration, phases => \%phases,
        module_sizes => {map {($_ == 0 ? 'cppgm' : 'clang') => module_sizes($modules[$_])} (0,1)},
        module_sha256 => {map {($_ == 0 ? 'cppgm' : 'clang') => digest($modules[$_])} (0,1)},
        cases => \@cases, case_set_sha256 => sha256_hex($json->encode(\@cases))};
    print "$configuration: checked " . scalar(@cases) . " independent cases per compiler\n";
}
save_json("$output/results.json", {schema => 2, status => 'complete', metadata => $metadata,
    configurations => \@configurations, warnings => \@warnings, blocks => $blocks, aa_blocks => $aa_blocks});
my $details = "# Wasm32 benchmark\n\nCompiler: `$revision`; $cpu; $metadata->{optimization}.\n\n" .
    "Reference / cppgm; higher is better. 64 independent oracle cases pass per compiler/configuration.\n\n" .
    "| Configuration | Phase / metric | cppgm | Clang | Ratio |\n|---|---|---:|---:|---:|\n";
for my $config (@configurations) {
    for my $phase ('compile','link','startup',@names) {
        for my $metric ($phase eq 'startup' ? ('seconds') : ('cpu_seconds','seconds')) {
            my $v = $config->{phases}{$phase}{AB}{$metric};
            $details .= sprintf("| %s | %s / %s | %.6f | %.6f | %.3fx |\n",
                $config->{id},$phase,$metric,$v->{candidate},$v->{reference},$v->{speedup});
        }
    }
    for my $size ('module','code','data') {
        my $s = $config->{module_sizes};
        $details .= "| $config->{id} | $size / bytes | $s->{cppgm}{$size} | $s->{clang}{$size} | " .
            ($s->{cppgm}{$size} ? sprintf('%.3fx', $s->{clang}{$size}/$s->{cppgm}{$size}) : 'n/a') . " |\n";
    }
}
$details .= "\nCautions:\n\n" . join('', map {"- $_\n"} @warnings) if @warnings;
$details .= "\nFlags, engine configuration, hashes, every observation and paired ratios are retained alongside this report.\n";
write_file("$output/details.md", $details);
print "Wasm benchmark complete; raw observations in $output\n";
