package CppgmWasmQuality;

use strict;
use warnings;
use Exporter 'import';
use CppgmWasmModule;
our @EXPORT_OK = qw(check_quality);

sub byte { return CppgmWasmModule::byte($_[0]); }
sub number { return CppgmWasmModule::number($_[0]); }
sub bytes { return CppgmWasmModule::bytes(@_); }

sub leb {
    my ($reader, $limit) = @_;
    for (1 .. $limit) { return unless byte($reader) & 128; }
    die "Oversized Wasm quality immediate\n";
}

# Decode immediate lengths, not instruction selections or stackification.
# Independent module validation runs before this structural measurement.
sub function_metrics {
    my ($data) = @_;
    my $r = {data => $data, offset => 0};
    my %metrics = (code_bytes => length($data), locals => 0, instructions => 0,
        memory_ops => 0, zero_offset_memory_ops => 0, bulk_memory_ops => 0, control_depth => 0);
    for (1 .. number($r)) {
        $metrics{locals} += number($r);
        byte($r);
    }
    my $depth = 0;
    while ($r->{offset} < length($data)) {
        my $op = byte($r);
        ++$metrics{instructions};
        if ($op == 2 || $op == 3 || $op == 4 || $op == 0x1f) {
            leb($r, 5);
            ++$depth;
            $metrics{control_depth} = $depth if $depth > $metrics{control_depth};
            if ($op == 0x1f) {
                for (1 .. number($r)) {
                    my $kind = byte($r);
                    number($r) if $kind < 2;
                    number($r);
                }
            }
        } elsif ($op == 0x0b) {
            --$depth;
        } elsif ($op == 8 || $op == 0x0c || $op == 0x0d || $op == 0x10 ||
                 ($op >= 0x20 && $op <= 0x26) || $op == 0xd2) {
            number($r);
        } elsif ($op == 0x0e) {
            number($r) for 1 .. number($r) + 1;
        } elsif ($op == 0x11) {
            number($r) for 1 .. 2;
        } elsif ($op == 0x1c) {
            bytes($r, number($r));
        } elsif ($op >= 0x28 && $op <= 0x3e) {
            ++$metrics{memory_ops};
            number($r); # alignment
            ++$metrics{zero_offset_memory_ops} if number($r) == 0;
        } elsif ($op == 0x3f || $op == 0x40) {
            ++$metrics{memory_ops};
            number($r);
        } elsif ($op == 0x41 || $op == 0x42 || $op == 0xd0) {
            leb($r, $op == 0x42 ? 10 : 5);
        } elsif ($op == 0x43 || $op == 0x44) {
            bytes($r, $op == 0x43 ? 4 : 8);
        } elsif ($op == 0xfc) {
            my $sub = number($r);
            die "Unsupported bulk operation in quality measurement\n" if $sub > 17;
            if ($sub == 8 || $sub == 10 || $sub == 11) {
                ++$metrics{memory_ops};
                ++$metrics{bulk_memory_ops};
            }
            number($r) if $sub >= 8;
            number($r) if $sub == 8 || $sub == 10 || $sub == 12 || $sub == 14;
        } elsif ($op == 0xfd) {
            my $sub = number($r);
            die "Unsupported SIMD extension in quality measurement\n" if $sub > 255;
            if ($sub <= 11 || ($sub >= 84 && $sub <= 93)) {
                ++$metrics{memory_ops};
                number($r); # alignment
                ++$metrics{zero_offset_memory_ops} if number($r) == 0;
                byte($r) if $sub >= 84 && $sub <= 91;
            } elsif ($sub == 12 || $sub == 13) {
                bytes($r, 16);
            } elsif ($sub >= 21 && $sub <= 34) {
                byte($r);
            }
        } elsif (!(($op >= 0x45 && $op <= 0xc4) || $op == 0 || $op == 1 ||
                   $op == 5 || $op == 0x0a || $op == 0x0f || $op == 0x1a ||
                   $op == 0x1b || $op == 0xd1)) {
            die sprintf("Unsupported opcode 0x%x in quality measurement\n", $op);
        }
    }
    die "Unbalanced quality control stream\n" if $depth != -1;
    return \%metrics;
}

sub check_quality {
    my ($path, $quality, $level) = @_;
    return unless grep { $_ == $level } @{$quality->{levels} // [1, 2, 3]};
    if ($quality->{module}) {
        my $actual = CppgmWasmModule::module_sizes($path);
        check_bounds($actual, $quality->{module}, "$path: O$level module");
    }
    my (undef, undef, $module) = CppgmWasmModule::export_interfaces($path, 1);
    my @bodies;
    for my $section (@{$module->{sections}}) {
        next unless $section->[0] == 10;
        my $r = {data => $section->[1], offset => 0};
        push @bodies, bytes($r, number($r)) for 1 .. number($r);
    }
    my $imports = @{$module->{functions}} - @bodies;
    for my $name (sort keys %{$quality->{functions} // {}}) {
        die "Missing quality export $name\n" unless exists $module->{exports}{$name};
        my $index = $module->{exports}{$name} - $imports;
        die "Quality export $name has no body\n" if $index < 0 || $index >= @bodies;
        my $actual = function_metrics($bodies[$index]);
        check_bounds($actual, $quality->{functions}{$name}, "$path: O$level $name");
    }
}

sub check_bounds {
    my ($actual, $bounds, $where) = @_;
    for my $metric (sort keys %$bounds) {
        die "Unknown Wasm quality metric $metric\n" unless exists $actual->{$metric};
        my $limit = $bounds->{$metric};
        die "Invalid Wasm quality bound $metric\n" unless defined($limit) && $limit =~ /^\d+$/;
        die "$where $metric $actual->{$metric} exceeds $limit\n" if $actual->{$metric} > $limit;
    }
}

1;
