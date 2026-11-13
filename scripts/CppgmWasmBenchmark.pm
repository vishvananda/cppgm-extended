package CppgmWasmBenchmark;

use strict;
use warnings;
use Exporter 'import';
use CppgmWasmModule;
use CppgmWasmApplications;
our @EXPORT_OK = qw(oracle module_sizes summarize median);

sub mul32 {
    my ($a, $b) = @_;
    return (($a & 65535) * ($b & 65535) +
        ((($a >> 16) * ($b & 65535) + ($a & 65535) * ($b >> 16)) << 16)) & 0xffffffff;
}

sub rotate {
    my ($a, $n) = @_;
    return (($a << $n) | ($a >> ((32 - $n) & 31))) & 0xffffffff;
}

sub oracle {
    my ($kind, $repeats, $seed) = @_;
    my (@input, $state);
    $state = $seed;
    for (1 .. 2048) {
        $state = (mul32($state, 1664525) + 1013904223) & 0xffffffff;
        push @input, $state;
    }
    my $result = 17;
    for my $r (0 .. $repeats - 1) {
        my $key = ($seed + $r) & 0xffffffff;
        my $value = 0;
        if ($kind == 0) {
            $value = $key;
            for my $i (0 .. 2047) {
                $value = (mul32(rotate($value ^ $input[$i], 7), 16777619) + $i) & 0xffffffff;
            }
        } elsif ($kind == 1) {
            for my $x (@input) {
                $value = ($value + ($x ^ $key)) & 0xffffffff if ($x & 1023) < ($key & 1023);
            }
        } elsif ($kind == 2) {
            for my $i (0 .. 15) {
                for my $j (0 .. 15) {
                    my $sum = $key;
                    for my $k (0 .. 15) {
                        $sum = ($sum + mul32($input[$i*16+$k], $input[256+$k*16+$j])) & 0xffffffff;
                    }
                    $value ^= rotate($sum, ($i+$j) & 7);
                }
            }
        } elsif ($kind == 3) {
            for my $i (0 .. 2047) {
                my $x = $input[$i];
                my $y = ((mul32($x, 3) + $key) & 0xffffffff) ^ (($x >> 5) + 11 + ($i & 3)*6);
                $value = ($value + $y) & 0xffffffff;
            }
        } else { $value = CppgmWasmApplications::value($kind, $seed, $key); }
        $result = rotate($result, 5) ^ $value;
    }
    return $result;
}

sub module_sizes {
    return CppgmWasmModule::module_sizes(@_);
}

sub median {
    my @values = sort {$a <=> $b} @_;
    die "Empty measurements\n" unless @values;
    return ($values[int($#values/2)] + $values[int(@values/2)]) / 2;
}

sub summarize {
    my ($rows, $metric) = @_;
    my (@candidate, @reference, @ratios);
    die "Incomplete paired blocks\n" if @$rows % 4;
    for (my $i = 0; $i < @$rows; $i += 4) {
        my @block = @$rows[$i .. $i+3];
        die "Wrong ABBA order\n" unless join('', map {$_->{slot}} @block) eq '0110';
        my ($a, $b) = (($block[0]{$metric}+$block[3]{$metric})/2,
                       ($block[1]{$metric}+$block[2]{$metric})/2);
        die "Nonpositive measurement\n" unless $a > 0 && $b > 0;
        push @ratios, $b / $a;
        push @candidate, map {$_->{$metric}} @block[0,3];
        push @reference, map {$_->{$metric}} @block[1,2];
    }
    return {speedup => median(@ratios), ratios => \@ratios,
        candidate => median(@candidate), reference => median(@reference)};
}

1;
