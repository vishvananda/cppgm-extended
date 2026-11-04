# Reading a wider lane observes the zero bits of a scalar insertion too.
global @poison : obj<16x16> = { i64 -6510615557136195584 i64 -1 }
global @output : obj<32x16> = { zero 32 }

function @unpack(%source : ptr) -> i64 {
  block ^entry:
    %packed = load v128 %source
    %scalar = vector tof32 %packed
    %inserted = vector fromf32 %scalar
    %joined = vector unpcklpd %inserted, 0
    %bits = vector toi64 %joined
    return i64 %bits
}

function @mixed_min(%source : ptr) -> f32 {
  block ^entry:
    %packed = load v128 %source
    %scalar = vector tof32 %packed
    %inserted = vector fromf32 %scalar
    %minimum = vector minpd %inserted, 0
    %result = vector tof32 %minimum
    return f32 %result
}

function @multiple_uses(%source : ptr, %out : ptr) -> void {
  block ^entry:
    %packed = load v128 %source
    %scalar = vector tof32 %packed
    %inserted = vector fromf32 %scalar
    %shuffled = vector pshufd %inserted, 0
    store v128 %inserted, %out
    %next = index i8 %out, 16
    store v128 %shuffled, %next
    return void
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %source = addr @poison
    %out = addr @output
    %bits = call i64 @unpack(%source)
    %expected = load u32 %source
    %wide = convert zext i64 u32 %expected
    %bad_unpack = cmp ne i64 %bits, %wide
    %minimum = call f32 @mixed_min(%source)
    store f32 %minimum, %out
    %minimum_bits = load i32 %out
    %bad_minimum = cmp ne i32 %minimum_bits, 0
    call void @multiple_uses(%source, %out)
    %original = load i64 %out
    %bad_original = cmp ne i64 %original, %wide
    %high_place = index i8 %out, 8
    %high = load i64 %high_place
    %bad_high = cmp ne i64 %high, 0
    %bad0 = binary or i64 %bad_unpack, %bad_minimum
    %bad1 = binary or i64 %bad_original, %bad_high
    %bad = binary or i64 %bad0, %bad1
    return i64 %bad
}
