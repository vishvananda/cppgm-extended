package CppgmWasmModule;

use strict;
use warnings;
use Exporter 'import';
our @EXPORT_OK = qw(export_signatures export_tag_signatures);

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
    my %names = (0x7f => 'i32', 0x7e => 'i64', 0x7d => 'f32', 0x7c => 'f64', 0x7b => 'v128', 0x69 => 'exnref');
    return [map { $names{byte($reader)} // die "Unsupported ABI value type\n" }
        1 .. number($reader)];
}

sub export_interfaces {
    my ($path) = @_;
    open(my $file, '<:raw', $path) or die "Cannot read $path: $!\n";
    local $/;
    my $reader = {data => scalar(<$file>), offset => 0};
    bytes($reader, 8) eq "\0asm\1\0\0\0" or die "Expected core Wasm module\n";
    my (@types, @functions, @tags, %exports, %tag_exports);
    while ($reader->{offset} < length($reader->{data})) {
        my $id = byte($reader);
        my $section = {data => bytes($reader, number($reader)), offset => 0};
        if ($id == 1) {
            for (1 .. number($section)) {
                byte($section) == 0x60 or die "Expected core function type\n";
                push @types, {params => value_types($section), results => value_types($section)};
            }
        } elsif ($id == 2) {
            for (1 .. number($section)) {
                bytes($section, number($section)) for 1 .. 2;
                my $kind = byte($section);
                if ($kind == 0) { push @functions, number($section); }
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
    return (\%result, \%tag_result);
}

sub export_signatures { my ($functions, $tags) = export_interfaces(@_); return $functions; }
sub export_tag_signatures { my ($functions, $tags) = export_interfaces(@_); return $tags; }

1;
