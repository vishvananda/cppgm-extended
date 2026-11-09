# Discard an intermediate widening, preserving its sign behavior if the
# final type is wider than the original and preserving shared readers.
function @drop_middle(%x : u32) -> u8 [no_inline=yes] {
  block ^entry:
    %wide = convert zext i64 u32 %x
    %byte = convert trunc u8 i64 %wide
    return u8 %byte
}
function @keep_sign(%x : i8) -> i32 [no_inline=yes] {
  block ^entry:
    %wide = convert sext i64 i8 %x
    %narrow = convert trunc exact i32 i64 %wide
    return i32 %narrow
}
function @keep_zero(%x : u8) -> u32 [no_inline=yes] {
  block ^entry:
    %wide = convert zext i64 u8 %x
    %narrow = convert trunc u32 i64 %wide
    return u32 %narrow
}
function @retype(%x : i32) -> u32 [no_inline=yes] {
  block ^entry:
    %wide = convert sext i64 i32 %x
    %narrow = convert trunc u32 i64 %wide
    return u32 %narrow
}
function @shared(%x : u32, %out : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %wide = convert zext i64 u32 %x
    %byte = convert trunc u8 i64 %wide
    store u8 %byte, %out
    %sum = binary add i64 %wide, 4294967296
    return i64 %sum
}
function @wide_sign(%x : i8) -> i64 [no_inline=yes] {
  block ^entry:
    %wide = convert sext i128 i8 %x
    %result = convert trunc exact i64 i128 %wide
    return i64 %result
}
function @wide_byte(%x : i64) -> u8 [no_inline=yes] {
  block ^entry:
    %wide = convert zext i128 i64 %x
    %result = convert trunc u8 i128 %wide
    return u8 %result
}
function @main() -> i64 [role=entry] {
  slot $out : u8
  block ^entry:
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^next: %inext]
    %shifted = binary shl i64 %i, 24
    %pattern = binary or i64 %shifted, %i
    %word = convert trunc u32 i64 %pattern
    %sbyte = convert trunc i8 i64 %i
    %ubyte = convert trunc u8 i64 %i
    %negative = binary sub i64 %i, 512
    %signed_word = convert trunc i32 i64 %negative
    %a8 = call u8 @drop_middle(%word)
    %a = convert zext i64 u8 %a8
    %b32 = call i32 @keep_sign(%sbyte)
    %b = convert sext i64 i32 %b32
    %c32 = call u32 @keep_zero(%ubyte)
    %c = convert zext i64 u32 %c32
    %d32 = call u32 @retype(%signed_word)
    %d = convert zext i64 u32 %d32
    %p = addr $out
    %e = call i64 @shared(%word, %p)
    %wide_signed = call i64 @wide_sign(%sbyte)
    %wide_byte8 = call u8 @wide_byte(%negative)
    %wide_byte64 = convert zext i64 u8 %wide_byte8
    %out8 = load u8 $out
    %out64 = convert zext i64 u8 %out8
    %byte = binary and i64 %i, 255
    %sign = cmp uge i64 %byte, 128
    %extended = binary sub i64 %byte, 256
    %expected_sign = select i64 %sign, %extended, %byte
    %expected_word = binary and i64 %negative, 4294967295
    %expected_low = binary and i64 %pattern, 4294967295
    %expected_sum = binary add i64 %expected_low, 4294967296
    %bad_a = cmp ne i64 %a, %byte
    %bad_b = cmp ne i64 %b, %expected_sign
    %bad_c = cmp ne i64 %c, %byte
    %bad_d = cmp ne i64 %d, %expected_word
    %bad_e = cmp ne i64 %e, %expected_sum
    %bad_f = cmp ne i64 %out64, %byte
    %bad_g = cmp ne i64 %wide_signed, %expected_sign
    %bad_h = cmp ne i64 %wide_byte64, %byte
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %ef = binary or i64 %bad_e, %bad_f
    %gh = binary or i64 %bad_g, %bad_h
    %abcd = binary or i64 %ab, %cd
    %abcdef = binary or i64 %abcd, %ef
    %bad = binary or i64 %abcdef, %gh
    branch %bad, ^fail, ^next
  block ^next:
    %inext = binary add i64 %i, 1
    %more = cmp ne i64 %inext, 1024
    branch %more, ^loop, ^done
  block ^fail:
    return i64 1
  block ^done:
    return i64 0
}
