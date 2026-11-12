package CppgmWasmGlobals;

use strict;
use warnings;
use Exporter 'import';
use CppgmWasmModule;
our @EXPORT_OK = qw(initialize_globals metadata_first group_object);

sub number { return CppgmWasmModule::number($_[0]); }
sub byte { return CppgmWasmModule::byte($_[0]); }
sub bytes { return CppgmWasmModule::bytes(@_); }
sub uleb { return CppgmWasmModule::unsigned_leb($_[0]); }
sub string { return bytes($_[0], number($_[0])); }

sub symbols {
    my ($data, $no_sections) = @_;
    my $r = {data => $data, offset => 0};
    string($r) eq 'linking' && number($r) == 2 or die "Expected linking-v2 metadata\n";
    while ($r->{offset} < length($data)) {
        my $id = byte($r);
        my $s = {data => bytes($r, number($r)), offset => 0};
        next unless $id == 8;
        my %symbols;
        my $count = number($s);
        for my $ordinal (0 .. $count - 1) {
            my $kind = byte($s);
            die "Metadata reorder fixture cannot contain section symbols\n" if $no_sections && $kind == 3;
            my $flags = number($s);
            my ($index, $name);
            if ($kind == 1) {
                $name = string($s);
                $index = number($s) if !($flags & 16);
                number($s) for 1 .. (($flags & 16) ? 0 : 2);
            } else {
                $index = number($s);
                $name = string($s) if $kind != 3 && (!($flags & 16) || ($flags & 64));
            }
            $symbols{$name} = {kind => $kind, index => $index, ordinal => $ordinal}
                if defined($name) && !($flags & 16);
        }
        return \%symbols;
    }
    die "Missing reference symbol table\n";
}

sub sections {
    my ($data) = @_;
    substr($data, 0, 8) eq "\0asm\1\0\0\0" or die "Expected core object\n";
    my $r = {data => $data, offset => 8};
    my @out;
    while ($r->{offset} < length($data)) {
        my $id = byte($r);
        push @out, [$id, bytes($r, number($r))];
    }
    return @out;
}

sub module_bytes {
    return "\0asm\1\0\0\0" . join('', map { chr($_->[0]) . uleb(length($_->[1])) . $_->[1] } @_);
}

sub metadata_first {
    my ($data) = @_;
    my @sections = sections($data);
    my @order = ((grep { $sections[$_][0] == 0 } 0 .. $#sections),
                 (grep { $sections[$_][0] != 0 } 0 .. $#sections));
    my @indices;
    $indices[$order[$_]] = $_ for 0 .. $#order;
    for my $section (@sections) {
        next unless $section->[0] == 0;
        my $r = {data => $section->[1], offset => 0};
        my $name = string($r);
        symbols($section->[1], 1) if $name eq 'linking';
        if ($name =~ /^reloc\./) {
            my $prefix = $r->{offset};
            my $target = number($r);
            defined($indices[$target]) or die "Invalid fixture relocation section\n";
            $section->[1] = substr($r->{data}, 0, $prefix) . uleb($indices[$target]) .
                substr($r->{data}, $r->{offset});
        }
    }
    return module_bytes(@sections[@order]);
}

sub group_object {
    my ($data, $groups) = @_;
    my @sections = sections($data);
    for my $section (@sections) {
        next unless $section->[0] == 0;
        my $r = {data => $section->[1], offset => 0};
        next unless string($r) eq 'linking';
        my $symbols = symbols($section->[1]);
        my $payload = uleb(scalar keys %$groups);
        for my $name (sort keys %$groups) {
            $payload .= uleb(length($name)) . $name . "\x00" . uleb(scalar @{$groups->{$name}});
            for my $entry (@{$groups->{$name}}) {
                my ($kind, $member) = @$entry;
                my $symbol = $symbols->{$member} // die "Unknown COMDAT fixture member\n";
                my $symbol_kind = $kind eq 'function' ? 0 : $kind eq 'global' ? 2 : -1;
                $symbol->{kind} == $symbol_kind or die "Wrong COMDAT fixture member kind\n";
                $payload .= chr($kind eq 'function' ? 1 : 2) . uleb($symbol->{index});
            }
        }
        $section->[1] .= "\x07" . uleb(length($payload)) . $payload;
        return module_bytes(@sections);
    }
    die "Reference COMDAT fixture lacks linking metadata\n";
}

# LLVM's assembler supplies zero global initializers. Replace only those
# typed fields with published core constant expressions and standard relocations.
sub initialize_globals {
    my ($data, $initializers) = @_;
    substr($data, 0, 8) eq "\0asm\1\0\0\0" or die "Expected core object\n";
    my $r = {data => $data, offset => 8};
    my (@sections, $global_section, $symbols);
    while ($r->{offset} < length($data)) {
        my $id = byte($r);
        my $payload = bytes($r, number($r));
        push @sections, [$id, $payload];
        $global_section = $#sections if $id == 6;
        if ($id == 0) {
            my $c = {data => $payload, offset => 0};
            my $name = string($c);
            $symbols = symbols($payload) if $name eq 'linking';
            die "Reference template already has global relocations\n" if $name eq 'reloc.GLOBAL';
        }
    }
    defined($global_section) && $symbols or die "Reference template lacks globals or symbols\n";
    my $globals = {data => $sections[$global_section][1], offset => 0};
    my $count = number($globals);
    my @names;
    for my $name (keys %$symbols) {
        my $symbol = $symbols->{$name};
        $names[$symbol->{index}] = $name if $symbol->{kind} == 2;
    }
    my %types = (i32 => 0x7f, i64 => 0x7e, f32 => 0x7d, f64 => 0x7c,
                 v128 => 0x7b, funcref => 0x70, externref => 0x6f, exnref => 0x69);
    my %binary = ('i32.add' => 0x6a, 'i32.sub' => 0x6b, 'i32.mul' => 0x6c,
                  'i64.add' => 0x7c, 'i64.sub' => 0x7d, 'i64.mul' => 0x7e);
    my ($output, @relocations, %used) = (uleb($count));
    for my $index (0 .. $count - 1) {
        my $type = byte($globals);
        my $mutability = byte($globals);
        my $start = $globals->{offset};
        my $opcode = byte($globals);
        if ($opcode == 0x41 || $opcode == 0x42) {
            1 while byte($globals) & 128;
        } elsif ($opcode == 0x43 || $opcode == 0x44) {
            bytes($globals, $opcode == 0x43 ? 4 : 8);
        } else { die "Reference template needs numeric constant initializers\n"; }
        byte($globals) == 0x0b or die "Unexpected reference initializer\n";
        my $original = substr($globals->{data}, $start, $globals->{offset} - $start);
        my $name = $names[$index] // die "Reference template needs named globals without global imports\n";
        my $replacement = $initializers->{$name};
        if (!$replacement) { $output .= chr($type) . chr($mutability) . $original; next; }
        $used{$name} = 1;
        $output .= chr($types{$replacement->{type}} // die "Unknown global fixture type\n") . chr($mutability);
        for my $instruction (@{$replacement->{init}}) {
            my ($op, @operands) = @$instruction;
            if ($op =~ /^(i32|i64|f32|f64)\.const$/) {
                $output .= CppgmWasmModule::scalar_constant($1, $operands[0]);
            } elsif ($op eq 'ref.null') {
                my $heap = $types{$operands[0]} // die "Unknown fixture reference type\n";
                $output .= "\xd0" . substr(CppgmWasmModule::scalar_constant('i32', $heap - 128), 1);
            } elsif ($op eq 'v128.const') {
                @operands == 16 or die "Vector fixture constant needs 16 bytes\n";
                $output .= "\xfd\x0c" . pack('C*', @operands);
            } elsif ($op eq 'global.get' || $op eq 'ref.func') {
                my $symbol = $symbols->{$operands[0]} // die "Unknown fixture symbol\n";
                $symbol->{kind} == ($op eq 'ref.func' ? 0 : 2) or die "Fixture relocation symbol kind mismatch\n";
                $output .= chr($op eq 'ref.func' ? 0xd2 : 0x23);
                push @relocations, [$op eq 'ref.func' ? 0 : 7, length($output), $symbol->{ordinal}];
                my $index = $symbol->{index};
                $output .= pack('C*', map { (($index >> (7 * $_)) & 127) | ($_ == 4 ? 0 : 128) } 0 .. 4);
            } elsif (exists $binary{$op}) { $output .= chr($binary{$op}); }
            else { die "Unsupported fixture constant expression $op\n"; }
        }
        $output .= "\x0b";
    }
    keys(%used) == keys(%$initializers) or die "Unknown reference initializer name\n";
    $sections[$global_section][1] = $output;
    if (@relocations) {
        my $name = 'reloc.GLOBAL';
        my $payload = uleb(length($name)) . $name . uleb($global_section) . uleb(scalar @relocations);
        $payload .= join('', map { join('', map { uleb($_) } @$_) } @relocations);
        push @sections, [0, $payload];
    }
    return substr($data, 0, 8) . join('', map { chr($_->[0]) . uleb(length($_->[1])) . $_->[1] } @sections);
}

1;
