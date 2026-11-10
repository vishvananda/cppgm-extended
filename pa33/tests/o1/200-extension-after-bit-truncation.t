# An extension after truncation must respect the bit width, including i1.
# Sign extension observes its sign bit; zero extension keeps that bit.
function @sign_truth(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %truth = cmp ne i64 %x, 0
    %bit = convert trunc i1 i64 %truth
    %wide = convert sext i64 i1 %bit
    return i64 %wide
}
function @zero_bit(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %masked = binary and i64 %x, 3
    %bit = convert trunc i1 i64 %masked
    %wide = convert zext i64 i1 %bit
    return i64 %wide
}
function @zero_truth(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %truth = cmp ne i64 %x, 0
    %bit = convert trunc i1 i64 %truth
    %wide = convert zext i64 i1 %bit
    return i64 %wide
}
function @zero_byte(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %masked = binary and i64 %x, 255
    %byte = convert trunc u8 i64 %masked
    %wide = convert zext i64 u8 %byte
    return i64 %wide
}
function @sign_byte(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %masked = binary and i64 %x, 255
    %byte = convert trunc i8 i64 %masked
    %wide = convert sext i64 i8 %byte
    return i64 %wide
}
function @main() -> i64 [role=entry] {
  block ^entry:
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^next: %inext]
    %a = call i64 @sign_truth(%i)
    %b = call i64 @zero_bit(%i)
    %c = call i64 @zero_truth(%i)
    %d = call i64 @zero_byte(%i)
    %e = call i64 @sign_byte(%i)
    %truth = cmp ne i64 %i, 0
    %negative_truth = binary sub i64 0, %truth
    %low_bit = binary and i64 %i, 1
    %low_byte = binary and i64 %i, 255
    %negative_byte = binary sub i64 %low_byte, 256
    %sign = cmp uge i64 %low_byte, 128
    %signed_byte = select i64 %sign, %negative_byte, %low_byte
    %bad_a = cmp ne i64 %a, %negative_truth
    %bad_b = cmp ne i64 %b, %low_bit
    %bad_c = cmp ne i64 %c, %truth
    %bad_d = cmp ne i64 %d, %low_byte
    %bad_e = cmp ne i64 %e, %signed_byte
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %abcd = binary or i64 %ab, %cd
    %bad = binary or i64 %abcd, %bad_e
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
