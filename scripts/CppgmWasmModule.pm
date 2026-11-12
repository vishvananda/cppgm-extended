package CppgmWasmModule;

use strict;
use warnings;
use Exporter 'import';
our @EXPORT_OK = qw(export_signatures export_tag_signatures sequence_module);

# Read only the public core-module type/index/export sections. Instruction
# choices, local allocation and control structure are deliberately unobserved.
sub byte {
    my ($reader) = @_;
    die "Truncated Wasm module\n" if $reader->{offset} >= length($reader->{data});
    return ord(substr($reader->{data}, $reader->{offset}++, 1));
}

sub number {
    my ($reader) = @_;
    my $value = 0;
    for my $shift (0, 7, 14, 21, 28) {
        my $b = byte($reader);
        $value |= ($b & 127) << $shift;
        return $value if !($b & 128) && $value <= 0xffffffff;
    }
    die "Invalid Wasm u32\n";
}

sub bytes {
    my ($reader, $size) = @_;
    die "Truncated Wasm section\n" if $size > length($reader->{data}) - $reader->{offset};
    my $value = substr($reader->{data}, $reader->{offset}, $size);
    $reader->{offset} += $size;
    return $value;
}

sub limits {
    my ($reader) = @_;
    my $flags = number($reader);
    die "Unsupported shared/memory64 limits\n" if $flags > 1;
    number($reader);
    number($reader) if $flags & 1;
}

sub value_types {
    my ($reader) = @_;
    my %names = (0x7f => 'i32', 0x7e => 'i64', 0x7d => 'f32', 0x7c => 'f64',
                 0x7b => 'v128', 0x70 => 'funcref', 0x6f => 'externref', 0x69 => 'exnref');
    return [map { $names{byte($reader)} // die "Unsupported ABI value type\n" }
        1 .. number($reader)];
}

sub export_interfaces {
    my ($path, $keep_sections) = @_;
    open(my $file, '<:raw', $path) or die "Cannot read $path: $!\n";
    local $/;
    my $reader = {data => scalar(<$file>), offset => 0};
    bytes($reader, 8) eq "\0asm\1\0\0\0" or die "Expected core Wasm module\n";
    my (@types, @functions, @tags, %exports, %tag_exports, @sections, @function_imports);
    while ($reader->{offset} < length($reader->{data})) {
        my $id = byte($reader);
        my $section = {data => bytes($reader, number($reader)), offset => 0};
        push @sections, [$id, $section->{data}] if $keep_sections;
        if ($id == 1) {
            for (1 .. number($section)) {
                byte($section) == 0x60 or die "Expected core function type\n";
                push @types, {params => value_types($section), results => value_types($section)};
            }
        } elsif ($id == 2) {
            for (1 .. number($section)) {
                my ($module, $name) = map { bytes($section, number($section)) } 1 .. 2;
                my $kind = byte($section);
                if ($kind == 0) {
                    push @functions, number($section);
                    push @function_imports, [$module, $name];
                }
                elsif ($kind == 1) { byte($section); limits($section); }
                elsif ($kind == 2) { limits($section); }
                elsif ($kind == 3) { byte($section) for 1 .. 2; }
                elsif ($kind == 4) {
                    byte($section) == 0 or die "Unsupported tag attribute\n";
                    push @tags, number($section);
                }
                else { die "Unknown Wasm import kind\n"; }
            }
        } elsif ($id == 3) {
            push @functions, map { number($section) } 1 .. number($section);
        } elsif ($id == 13) {
            for (1 .. number($section)) {
                byte($section) == 0 or die "Unsupported tag attribute\n";
                push @tags, number($section);
            }
        } elsif ($id == 7) {
            for (1 .. number($section)) {
                my $name = bytes($section, number($section));
                my $kind = byte($section);
                my $index = number($section);
                $exports{$name} = $index if $kind == 0;
                $tag_exports{$name} = $index if $kind == 4;
            }
        }
    }
    my %result;
    for my $name (keys %exports) {
        my $index = $exports{$name};
        die "Invalid Wasm function export\n" if $index >= @functions || $functions[$index] >= @types;
        $result{$name} = $types[$functions[$index]];
    }
    my %tag_result;
    for my $name (keys %tag_exports) {
        my $index = $tag_exports{$name};
        die "Invalid Wasm tag export\n" if $index >= @tags || $tags[$index] >= @types;
        $tag_result{$name} = $types[$tags[$index]];
    }
    return (\%result, \%tag_result, {sections => \@sections, exports => \%exports,
        functions => \@functions, types => \@types, function_imports => \@function_imports});
}

sub module_sizes {
    my ($path) = @_;
    open(my $file, '<:raw', $path) or die "Cannot read $path: $!\n";
    local $/;
    my $reader = {data => scalar(<$file>), offset => 0};
    bytes($reader, 8) eq "\0asm\1\0\0\0" or die "Expected core module\n";
    my %size = (module => length($reader->{data}), code => 0, data => 0);
    while ($reader->{offset} < length($reader->{data})) {
        my $id = byte($reader);
        my $payload = number($reader);
        $size{code} += $payload if $id == 10;
        $size{data} += $payload if $id == 11;
        bytes($reader, $payload);
    }
    return \%size;
}

sub export_signatures { my ($functions, $tags) = export_interfaces(@_); return $functions; }
sub export_tag_signatures { my ($functions, $tags) = export_interfaces(@_); return $tags; }

# Append a small assertion driver to the final module. Its calls share one
# instance and its original start function; no external compiler is involved.
sub unsigned_leb {
    my ($n) = @_;
    my $out = '';
    do {
        my $b = $n & 127;
        $n >>= 7;
        $out .= chr($b | ($n ? 128 : 0));
    } while ($n);
    return $out;
}

sub scalar_constant {
    my ($type, $value) = @_;
    if ($type eq 'i32' || $type eq 'i64') {
        require Math::BigInt;
        my $bits = $type eq 'i32' ? 32 : 64;
        my $n = Math::BigInt->new("$value");
        my $sign = Math::BigInt->new(2)->bpow($bits - 1);
        my $modulus = $sign->copy()->bmul(2);
        die "Invalid sequence integer $value for $type\n"
            if $n->is_nan() || $n->bcmp($sign->copy()->bneg()) < 0 || $n->bcmp($modulus) >= 0;
        $n->bsub($modulus) if $n->bcmp($sign) >= 0;
        my $out = chr($type eq 'i32' ? 0x41 : 0x42);
        while (1) {
            my $b = $n->copy()->bmod(128)->numify();
            $n->brsft(7);
            my $done = ($n->is_zero() && !($b & 64)) || ($n->is_neg() && $n->is_one('-') && ($b & 64));
            $out .= chr($b | ($done ? 0 : 128));
            last if $done;
        }
        return $out;
    }
    die "Invalid sequence floating value $value\n"
        unless "$value" =~ /\A[+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:e[+-]?[0-9]+)?\z/i;
    return chr(0x43) . pack('f<', $value) if $type eq 'f32';
    return chr(0x44) . pack('d<', $value) if $type eq 'f64';
    die "Unsupported sequence scalar type $type\n";
}

sub sequence_module {
    my ($input, $output, $calls, $initialize) = @_;
    my ($interfaces, $tags, $module) = export_interfaces($input, 1);
    my $name = '__cppgm_sequence_check';
    die "Sequence driver export already exists\n" if exists $interfaces->{$name};
    my $body = "\0"; # no locals
    if (defined $initialize) {
        my $signature = $interfaces->{$initialize} // die "Missing sequence initializer $initialize\n";
        die "Invalid sequence initializer type\n" if @{$signature->{params}} || @{$signature->{results}};
        $body .= chr(0x10) . unsigned_leb($module->{exports}{$initialize});
    }
    my %ne = (i32 => 0x47, i64 => 0x52, f32 => 0x5c, f64 => 0x62);
    my $ordinal = 0;
    for my $call (@$calls) {
        ++$ordinal;
        my $signature = $interfaces->{$call->{export}} // die "Missing sequence export $call->{export}\n";
        my @args = @{$call->{args} // []};
        die "Sequence argument count mismatch\n" if @args != @{$signature->{params}};
        for my $i (0 .. $#args) {
            $body .= scalar_constant($signature->{params}[$i], $args[$i]);
        }
        $body .= chr(0x10) . unsigned_leb($module->{exports}{$call->{export}});
        my @results = @{$signature->{results}};
        die "Sequence check requires a scalar result or void\n" if @results > 1;
        if (@results) {
            my $type = $results[0];
            die "Sequence result assertion is missing\n" unless exists $call->{result};
            die "Unsupported sequence result type $type\n" unless exists $ne{$type};
            $body .= scalar_constant($type, $call->{result}) . chr($ne{$type}) . "\x04\x40"
                . scalar_constant('i32', -$ordinal) . "\x0f\x0b";
        } else {
            die "Void sequence call has a result assertion\n" if exists $call->{result};
        }
    }
    $body .= "\x41\x11\x0b"; # return 17
    my %append = (
        1 => "\x60\x00\x01\x7f",
        3 => unsigned_leb(scalar @{$module->{types}}),
        7 => unsigned_leb(length($name)) . $name . "\0" . unsigned_leb(scalar @{$module->{functions}}),
        10 => unsigned_leb(length($body)) . $body,
    );
    my $bytes = "\0asm\1\0\0\0";
    for my $section (@{$module->{sections}}) {
        my ($id, $data) = @$section;
        if (exists $append{$id}) {
            my $reader = {data => $data, offset => 0};
            my $count = number($reader);
            $data = unsigned_leb($count + 1) . substr($data, $reader->{offset}) . delete($append{$id});
        }
        $bytes .= chr($id) . unsigned_leb(length($data)) . $data;
    }
    die "Sequence module lacks required sections\n" if keys %append;
    open(my $file, '>:raw', $output) or die "Cannot write $output: $!\n";
    print {$file} $bytes;
    close($file) or die "Cannot close $output: $!\n";
    return $name;
}

1;
