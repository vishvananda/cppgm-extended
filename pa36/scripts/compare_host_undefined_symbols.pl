#!/usr/bin/env perl
use strict;
use warnings;

use File::Temp qw(tempdir);
use Getopt::Long qw(GetOptions);
use Text::ParseWords qw(shellwords);

sub usage {
    die "Usage: compare_host_undefined_symbols.pl --obj <obj> [--src <src>] [--exact] [--match <regex>] [--optional-match <regex>]\n";
}

sub split_shell_words {
    my ($text) = @_;
    return () if !defined($text) || $text eq "";
    return shellwords($text);
}

sub infer_src_from_obj {
    my ($obj_path) = @_;
    if($obj_path =~ /^(.*)\.(?:my|ref|check)\.(\d+)\.o$/) {
        return "$1.t.$2";
    }
    die "unable to infer source path from object path: $obj_path\n";
}

sub run_capture {
    my (@cmd) = @_;
    open(my $fh, "-|", @cmd) or die "failed to run $cmd[0]: $!\n";
    local $/;
    my $out = <$fh>;
    close($fh) or die "command failed: @cmd\n";
    return defined($out) ? $out : "";
}

sub extract_undefined_map {
    my ($obj_path) = @_;
    my $nm_output = run_capture("nm", "-u", $obj_path);
    my @raw_symbols;
    for my $line (split(/\n/, $nm_output)) {
        my @fields = split(/\s+/, $line);
        push(@raw_symbols, $fields[-1]) if @fields;
    }
    return () if !@raw_symbols;

    my $demangled_output = run_capture("c++filt", @raw_symbols);
    my @demangled_symbols = split(/\n/, $demangled_output);
    my @pairs;
    for(my $i = 0; $i < @raw_symbols; ++$i) {
        push(@pairs, [$raw_symbols[$i], $demangled_symbols[$i] // ""]);
    }
    return @pairs;
}

sub collect_matching_symbols {
    my ($symbol_map_ref, $pattern) = @_;
    my %matches;
    for my $entry (@{$symbol_map_ref}) {
        my ($raw, $demangled) = @{$entry};
        if($demangled =~ /$pattern/) {
            $matches{$raw} = 1;
        }
    }
    return sort keys %matches;
}

sub compile_host_object {
    my ($src_path) = @_;
    my $host_cxx = $ENV{"CPPGM_HOST_CXX"} || $ENV{"CXX"} || "c++";
    my @host_cxx_argv = split_shell_words($host_cxx);
    my @stdlib_flags = split_shell_words($ENV{"CPPGM_STDLIB_FLAGS"} || "");
    my $tmpdir = tempdir(CLEANUP => 1);
    my $host_obj = "$tmpdir/host.o";
    my $src_dir = $src_path;
    $src_dir =~ s{/[^/]*$}{};
    $src_dir = "." if $src_dir eq $src_path;

    my @cmd = (
        @host_cxx_argv,
        "-std=gnu++11",
        @stdlib_flags,
        "-I",
        $src_dir,
        "-x",
        "c++",
        "-c",
        "-o",
        $host_obj,
        $src_path,
    );
    system(@cmd) == 0 or die "host compile failed: @cmd\n";
    return ($host_obj, $tmpdir);
}

sub sorted_set_difference {
    my ($left_ref, $right_ref) = @_;
    my %right = map { $_ => 1 } @{$right_ref};
    return sort grep { !$right{$_} } @{$left_ref};
}

sub arrays_equal {
    my ($left_ref, $right_ref) = @_;
    return 0 if @{$left_ref} != @{$right_ref};
    for(my $i = 0; $i < @{$left_ref}; ++$i) {
        return 0 if $left_ref->[$i] ne $right_ref->[$i];
    }
    return 1;
}

sub main {
    my $obj_path = "";
    my $src_path = "";
    my $exact = 0;
    my @matches;
    my @optional_matches;
    GetOptions(
        "obj=s" => \$obj_path,
        "src=s" => \$src_path,
        "exact" => \$exact,
        "match=s" => \@matches,
        "optional-match=s" => \@optional_matches,
    ) or usage();
    usage() if $obj_path eq "";

    $src_path = infer_src_from_obj($obj_path) if $src_path eq "";
    my ($host_obj, $host_tmpdir) = compile_host_object($src_path);
    my @host_map = extract_undefined_map($host_obj);
    my @obj_map = extract_undefined_map($obj_path);

    if($exact) {
        my %host_seen = map { $_->[0] => 1 } @host_map;
        my %obj_seen = map { $_->[0] => 1 } @obj_map;
        my @host_symbols = sort keys %host_seen;
        my @obj_symbols = sort keys %obj_seen;
        if(!arrays_equal(\@host_symbols, \@obj_symbols)) {
            print "undefined_symbol_set_mismatch\n";
            print "  host_only: ", join(", ", sorted_set_difference(\@host_symbols, \@obj_symbols)), "\n";
            print "  ours_only: ", join(", ", sorted_set_difference(\@obj_symbols, \@host_symbols)), "\n";
            return 1;
        }
    }

    for my $pattern (@matches) {
        my @host_symbols = collect_matching_symbols(\@host_map, $pattern);
        my @obj_symbols = collect_matching_symbols(\@obj_map, $pattern);
        if(!@host_symbols) {
            print "host_pattern_missing $pattern\n";
            return 1;
        }
        if(!arrays_equal(\@host_symbols, \@obj_symbols)) {
            print "symbol_mismatch $pattern\n";
            print "  host: ", join(", ", @host_symbols), "\n";
            print "  ours: ", join(", ", @obj_symbols), "\n";
            return 1;
        }
    }

    for my $pattern (@optional_matches) {
        my @host_symbols = collect_matching_symbols(\@host_map, $pattern);
        next if !@host_symbols;
        my @obj_symbols = collect_matching_symbols(\@obj_map, $pattern);
        if(!arrays_equal(\@host_symbols, \@obj_symbols)) {
            print "symbol_mismatch $pattern\n";
            print "  host: ", join(", ", @host_symbols), "\n";
            print "  ours: ", join(", ", @obj_symbols), "\n";
            return 1;
        }
    }

    return 0;
}

exit(main());
