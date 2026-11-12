package CppgmWasmFixture;

use strict;
use warnings;
use Exporter 'import';
use CppgmWasmModule;
our @EXPORT_OK = qw(mutate_object);

sub uleb {
    my ($n) = @_;
    my $out = '';
    do {
        my $part = $n & 127;
        $n >>= 7;
        $out .= chr($part | ($n ? 128 : 0));
    } while ($n);
    return $out;
}

sub empty_signature {
    my ($data) = @_;
    my $r = {data => $data, offset => 0};
    my $n = CppgmWasmModule::number($r);
    for my $i (0 .. $n - 1) {
        CppgmWasmModule::byte($r) == 0x60 or die "Expected function type\n";
        my $p = CppgmWasmModule::value_types($r);
        my $v = CppgmWasmModule::value_types($r);
        return $i if !@$p && !@$v;
    }
    die "Reference fixture needs an empty signature\n";
}

sub tag_symbol {
    my ($data, $wanted) = @_;
    my $r = {data => $data, offset => 0};
    CppgmWasmModule::bytes($r, CppgmWasmModule::number($r)) eq 'linking'
        or die "Expected linking metadata\n";
    CppgmWasmModule::number($r) == 2 or die "Expected linking version 2\n";
    while ($r->{offset} < length($data)) {
        my $kind = CppgmWasmModule::byte($r);
        my $s = {data => CppgmWasmModule::bytes($r, CppgmWasmModule::number($r)), offset => 0};
        next unless $kind == 8;
        my $count = CppgmWasmModule::number($s);
        for my $id (0 .. $count - 1) {
            my $type = CppgmWasmModule::byte($s);
            my $flags = CppgmWasmModule::number($s);
            my $name = '';
            if ($type == 1) {
                $name = CppgmWasmModule::bytes($s, CppgmWasmModule::number($s));
                CppgmWasmModule::number($s) for 1 .. (($flags & 16) ? 0 : 3);
            } else {
                CppgmWasmModule::number($s);
                $name = CppgmWasmModule::bytes($s, CppgmWasmModule::number($s))
                    if $type != 3 && (!($flags & 16) || ($flags & 64));
            }
            return $id if $type == 4 && $name eq $wanted;
        }
    }
    die "Reference fixture lacks tag $wanted\n";
}

sub resultful_tag_type {
    my ($data) = @_;
    my $r = {data => $data, offset => 0};
    my $count = CppgmWasmModule::number($r);
    my (@types, $changed);
    for (1 .. $count) {
        CppgmWasmModule::byte($r) == 0x60 or die "Expected function type\n";
        my $parameters = CppgmWasmModule::number($r);
        my $params = CppgmWasmModule::bytes($r, $parameters);
        my $results = CppgmWasmModule::number($r);
        my $values = CppgmWasmModule::bytes($r, $results);
        if (!$changed && $parameters == 1 && !$results) {
            $changed = 1;
            $results = 1;
            $values = "\x7f";
        }
        push @types, "\x60" . uleb($parameters) . $params . uleb($results) . $values;
    }
    $changed or die "Reference fixture lacks a result-free tag type\n";
    return uleb($count) . join('', @types);
}

sub mutate_object {
    my ($data, $kind) = @_;
    my $r = {data => $data, offset => 8};
    substr($data, 0, 8) eq "\0asm\1\0\0\0" or die "Expected core object\n";
    my (@sections, $code, $types, $relocations, $prefix, $linking);
    while ($r->{offset} < length($data)) {
        my $id = CppgmWasmModule::byte($r);
        my $payload = CppgmWasmModule::bytes($r, CppgmWasmModule::number($r));
        push @sections, [$id, $payload];
        $code = $#sections if $id == 10;
        $types = $#sections if $id == 1;
        if ($id == 0) {
            my $s = {data => $payload, offset => 0};
            my $name = CppgmWasmModule::bytes($s, CppgmWasmModule::number($s));
            if ($name eq 'reloc.CODE') {
                $relocations = $#sections;
                $prefix = $s->{offset};
            }
            $linking = $#sections if $name eq 'linking';
        }
    }
    defined($code) && defined($types) && defined($relocations)
        or die "Reference fixture lacks code, types or relocations\n";
    if ($kind =~ /^simd-/) {
        my %fields = (
            'simd-lane-range' => ["\xfd\x15\0", 2, 16],
            'simd-shuffle-range' => ["\xfd\x0d" . pack('C*', 0 .. 15), 2, 32],
            'simd-memory-alignment' => ["\xfd\0\x04\0", 2, 5],
        );
        my $field = $fields{$kind} // die "Unknown SIMD mutation $kind\n";
        my $offset = index($sections[$code][1], $field->[0]);
        die "Missing or ambiguous SIMD fixture field\n"
            if $offset < 0 || rindex($sections[$code][1], $field->[0]) != $offset;
        substr($sections[$code][1], $offset + $field->[1], 1) = chr($field->[2]);
    } elsif ($kind eq 'tag-with-result') {
        $sections[$types][1] = resultful_tag_type($sections[$types][1]);
    } elsif ($kind eq 'signed-i64-overflow') {
        my $needle = "\x42" . ("\x80" x 9) . "\x7f";
        my $at = index($sections[$code][1], $needle);
        $at >= 0 or die "Reference fixture lacks the wide signed literal\n";
        substr($sections[$code][1], $at + 10, 1) = "\x7e";
    } else {
        my $s = {data => $sections[$relocations][1], offset => $prefix};
        my $section = CppgmWasmModule::number($s);
        my $count = CppgmWasmModule::number($s);
        my (@entries, $changed);
        for (1 .. $count) {
            my $start = $s->{offset};
            my $type = CppgmWasmModule::number($s);
            my $offset = CppgmWasmModule::number($s);
            my $index = CppgmWasmModule::number($s);
            $type == 0 || $type == 6 || $type == 7 || $type == 10
                or die "Unexpected fixture relocation $type\n";
            my $entry = substr($s->{data}, $start, $s->{offset} - $start);
            if ($kind eq 'tag-payload-relocation' && $type == 10 && !$changed) {
                $changed = 1;
                defined($linking) or die "Reference fixture lacks linking metadata\n";
                $entry = uleb($type) . uleb($offset) .
                    uleb(tag_symbol($sections[$linking][1], 'unused_exception'));
            } elsif ($type == 6 && !$changed && $kind ne 'tag-payload-relocation') {
                $changed = 1;
                if ($kind eq 'block-type-relocation') {
                    $entry = uleb($type) . uleb($offset) . uleb(empty_signature($sections[$types][1]));
                } elsif ($kind eq 'missing-block-relocation') {
                    next;
                } elsif ($kind eq 'signed-block-overflow') {
                    substr($sections[$code][1], $offset + 4, 1) = "\x20";
                } else {
                    die "Unknown reference mutation $kind\n";
                }
            }
            push @entries, $entry;
        }
        $changed or die "Reference fixture lacks a block type relocation\n";
        $sections[$relocations][1] = substr($s->{data}, 0, $prefix) .
            uleb($section) . uleb(scalar(@entries)) . join('', @entries);
    }
    return substr($data, 0, 8) . join('', map { chr($_->[0]) . uleb(length($_->[1])) . $_->[1] } @sections);
}

1;
