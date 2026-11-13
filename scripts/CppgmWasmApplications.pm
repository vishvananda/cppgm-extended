package CppgmWasmApplications;
use strict;
use warnings;
use JSON::PP;
my (%inputs, %values);

sub mul { return CppgmWasmBenchmark::mul32(@_); }
sub input {
    my ($seed) = @_;
    return $inputs{$seed} if exists $inputs{$seed};
    my (@words, @records);
    my $state = $seed;
    for (1 .. 65536) {
        $state = (mul($state, 1664525) + 1013904223) & 0xffffffff;
        push @words, $state;
    }
    for my $i (0 .. 4095) {
        my $value = $words[$i] & 65535;
        push @records, '{"name":"item\\n","value":' . ($value & 1 ? '-' : '') . $value . '}' .
            (($i & 7) == 0 ? "\n" : '');
    }
    my $text = '{"items":[' . join(',', @records) . ']}';
    return $inputs{$seed} = [\@words, $text];
}

sub shortest_paths {
    my ($words, $key) = @_;
    # Deliberately use a quadratic reference, not the candidate's binary heap.
    my @distance = (0xffffffff) x 2048;
    my @done = (0) x 2048;
    $distance[$key & 2047] = 0;
    for (1 .. 2048) {
        my ($node, $best) = (-1, 0xffffffff);
        for my $i (0 .. 2047) {
            ($node, $best) = ($i, $distance[$i]) if !$done[$i] && $distance[$i] < $best;
        }
        die "Disconnected graph" if $node < 0;
        $done[$node] = 1;
        for my $edge (0 .. 7) {
            my $x = $words->[$node*8+$edge];
            my $next = $edge ? ($x >> 12) & 2047 : ($node+1) & 2047;
            my $candidate = $best + 1 + ($x & 255);
            $distance[$next] = $candidate if $candidate < $distance[$next];
        }
    }
    my $result = 0;
    $result = ($result + mul($distance[$_], $_+1)) & 0xffffffff for 0 .. 2047;
    return $result;
}

sub parse_document {
    my ($text, $key) = @_;
    my $document = decode_json($text);
    my $result = 0;
    my $token = sub { $result = (mul($result, 33) + (($_[0] & 0xffffffff) ^ $key)) & 0xffffffff; };
    my $string = sub {
        my $hash = 0;
        $hash = (mul($hash, 31) + $_) & 0xffffffff for unpack('C*', $_[0]);
        $token->($hash);
    };
    $token->(ord('{')); $string->('items'); $token->(ord('['));
    for my $record (@{$document->{items}}) {
        $token->(ord('{')); $string->('name'); $string->($record->{name});
        $string->('value'); $token->($record->{value}); $token->(ord('}'));
    }
    $token->(ord(']')); $token->(ord('}'));
    return $result;
}

sub compressed_size {
    my ($text) = @_;
    my @bytes = unpack('C*', $text);
    my %last;
    my ($at, $size) = (0, 0);
    while ($at < @bytes) {
        my $length = 0;
        if ($at + 2 < @bytes) {
            my $hash = ($bytes[$at]*251 + $bytes[$at+1]*11 + $bytes[$at+2]) & 4095;
            my $previous = $last{$hash};
            $last{$hash} = $at;
            if (defined($previous) && $at-$previous <= 65535) {
                ++$length while $length < 130 && $at+$length < @bytes &&
                    $bytes[$previous+$length] == $bytes[$at+$length];
            }
        }
        if ($length >= 3) { $size += 3; $at += $length; }
        else { $size += 2; ++$at; }
    }
    return $size;
}

sub value {
    my ($kind, $seed, $key) = @_;
    my $id = "$kind/$seed/$key";
    return $values{$id} if exists $values{$id};
    my ($words, $text) = @{input($seed)};
    my $value = 0;
    if ($kind == 4) {
        my @sorted = sort {$a <=> $b} map {$_ ^ $key} @$words;
        $value = ($value + mul($sorted[$_], $_+1)) & 0xffffffff for 0 .. $#sorted;
    } elsif ($kind == 5) {
        $value = shortest_paths($words, $key);
    } elsif ($kind == 6) {
        $value = parse_document($text, $key);
    } elsif ($kind == 7) {
        $value = $key;
        $value = (mul($value, 33) + $_) & 0xffffffff for unpack('C*', $text);
        $value ^= compressed_size($text);
    } else { die "Unknown application kind $kind"; }
    return $values{$id} = $value;
}
1;
