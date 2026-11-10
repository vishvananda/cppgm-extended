#!/usr/bin/perl
use strict;
use warnings;
use File::Temp qw(tempdir);
use FindBin;
use lib $FindBin::Bin;
use CppgmBatchWorker qw(run_command_capture);

die "Usage: check_native_carrier_fields.pl <lowir2native> <fixture> [lowiropt]\n" if @ARGV < 2 || @ARGV > 3;
my ($app, $test, $optimizer) = @ARGV;
my $directory = tempdir('cppgm-carrier-fields-XXXXXX', TMPDIR => 1, CLEANUP => 1);
my @eligible = qw(low_signed high_unsigned high_signed last_signed inner_unsigned
    unaligned_signed unaligned_unsigned last_half second_high both_fields branch_field);
for my $route (defined($optimizer) ? qw(raw optimized) : qw(raw)) {
    for my $level (qw(-O0 -O1 -O2 -O3)) {
        my $mir = "$directory/$route-$level.mir";
        my $program = "$directory/$route-$level.program";
        my $stderr = "$directory/stderr";
        my $stdout = "$directory/stdout";
        my $source = $test;
        if ($route eq 'optimized') {
            $source = "$directory/$level.lowir";
            my $status = run_command_capture(cmd => [$optimizer, $level, '-o', $source, $test],
                stdout => $stdout, stderr => $stderr, timeout => 60);
            die "$test $level: LowIR optimization failed\n" if $status;
        }
        my $status = run_command_capture(
            cmd => [$app, $level, '--dump-machine-ir', $mir, '-o', $program, $source],
            stdout => $stdout, stderr => $stderr, timeout => 60);
        if ($status) {
            open(my $log, '<', $stderr) or die "Cannot read $stderr: $!\n";
            local $/;
            die "$test $level: compilation failed\n" . <$log>;
        }
        $status = run_command_capture(cmd => [$program], stdout => $stdout,
            stderr => $stderr, timeout => 10);
        die "$test $level: program failed ($status)\n" if $status || -s $stdout;
        next if $level eq '-O0';
        open(my $input, '<', $mir) or die "Cannot read $mir: $!\n";
        local $/;
        my $text = <$input>;
        for my $name (@eligible) {
            my ($body) = $text =~ /^function \@\Q$name\E\b(.*?)(?=^function \@|\z)/msg;
            die "$test $level: missing $name\n" if !defined $body;
            my ($instructions) = $body =~ /^\s+block\b(.*)/ms;
            die "$test $level: $name still materializes or reloads its private object\n"
                if !defined($instructions) || $instructions =~ /\b(?:load|store)\.[a-z0-9]+\b|\bcopy_bytes\b/;
            die "$test $level: $name still reserves unused frame storage\n"
                if $body !~ /^\s+stack_size 0$/m;
        }
        my ($kept) = $text =~ /^function \@retained_frame\b(.*?)(?=^function \@|\z)/msg;
        die "$test $level: missing retained-frame control\n" if !defined $kept;
        my ($operations) = $kept =~ /^\s+block\b(.*)/ms;
        die "$test $level: missing retained-frame body\n" if !defined $operations;
        my $work = () = $operations =~ /^    (?!nop\b)\S/gm;
        die "$test $level: field extraction increased work while storage stays necessary ($work > 5)\n"
            if $work > 5;
    }
}

print "native carrier fields: PASS (4 levels; 11 storage predicates; retained-frame work bound)\n"
    unless $ENV{CPPGM_REPORT_QUIET};
