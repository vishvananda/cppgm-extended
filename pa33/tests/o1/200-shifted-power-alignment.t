# A full-width shift of one is a power of two on every defined execution.
# The argument need not have a separately inferred upper bound.
function @round_up(%x : i64, %n : u8) -> i64 [no_inline=yes] {
  block ^entry:
    %count = convert zext i64 u8 %n
    %alignment = binary shl i64 1, %count
    %nonzero = cmp ne i64 %x, 0
    %bias = convert zext i64 i1 %nonzero
    %start = binary sub i64 %x, %bias
    %quotient = binary udiv i64 %start, %alignment
    %ceiling = binary add i64 %quotient, %bias
    %result = binary mul i64 %ceiling, %alignment
    return i64 %result
}

# Sign extension can turn a narrow single bit into a negative denominator.
# Its unsigned representation has many set bits and must keep that value.
function @signed_byte(%x : i64, %n : i8) -> i64 [no_inline=yes] {
  block ^entry:
    %bit = binary shl i8 1, %n
    %divisor = convert sext i64 i8 %bit
    %result = binary udiv i64 %x, %divisor
    return i64 %result
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %a = call i64 @round_up(123, 3)
    %b = call i64 @round_up(0, 63)
    %c = call i64 @round_up(-1, 63)
    %d = call i64 @round_up(8, 0)
    %e = call i64 @signed_byte(-1, 7)
    %bad_a = cmp ne i64 %a, 128
    %bad_d = cmp ne i64 %d, 8
    %bad_e = cmp ne i64 %e, 1
    %ab = binary or i64 %bad_a, %b
    %cd = binary or i64 %c, %bad_d
    %abcd = binary or i64 %ab, %cd
    %bad = binary or i64 %abcd, %bad_e
    return i64 %bad
}
