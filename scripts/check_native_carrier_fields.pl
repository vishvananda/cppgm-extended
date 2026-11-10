#!/usr/bin/perl
use strict;
use warnings;
use File::Temp qw(tempdir);
use FindBin;
use lib $FindBin::Bin;
use CppgmBatchWorker qw(run_command_capture);

die "Usage: check_native_carrier_fields.pl <lowir2native> <fixture>\n" if @ARGV != 2;
my ($app, $test) = @ARGV;
my $directory = tempdir('cppgm-carrier-fields-XXXXXX', TMPDIR => 1, CLEANUP => 1);
my @eligible = qw(low_signed high_unsigned high_signed last_signed inner_unsigned
    unaligned_signed unaligned_unsigned last_half second_high both_fields branch_field);
for my $level (qw(-O0 -O1 -O2 -O3)) {
    my $mir = "$directory/$level.mir";
    my $program = "$directory/$level.program";
    my $stderr = "$directory/stderr";
    my $stdout = "$directory/stdout";
    my $status = run_command_capture(
        cmd => [$app, $level, '--dump-machine-ir', $mir, '-o', $program, $test],
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
}
print "native carrier fields: PASS (4 levels; 11 storage predicates)\n"
    unless $ENV{CPPGM_REPORT_QUIET};
