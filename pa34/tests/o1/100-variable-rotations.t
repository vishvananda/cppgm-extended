# Complementary variable logical shifts form a rotation. Counts and other
# live parameters must survive the x86 count-register constraint.
function @rotate_right(%x : i64, %count : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %n = binary and i64 %count, 63
    branch %n, ^rotate, ^zero
  block ^zero:
    return i64 %x
  block ^rotate:
    %other = binary sub i64 64, %n
    %a = binary ushr i64 %x, %n
    %b = binary shl i64 %x, %other
    %result = binary or i64 %a, %b
    return i64 %result
}
function @with_live_values(%x : i64, %a : i64, %b : i64, %count : i64,
                          %c : i64, %d : i64, %e : i64, %f : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %n = binary and i64 %count, 63
    branch %n, ^rotate, ^zero
  block ^zero:
    jump ^merge
  block ^rotate:
    %other = binary sub i64 64, %n
    %lo = binary ushr i64 %x, %n
    %hi = binary shl i64 %x, %other
    %rotated = binary or i64 %hi, %lo
    jump ^merge
  block ^merge:
    %value = phi i64 [^zero: %x, ^rotate: %rotated]
    %one = binary add i64 %value, %a
    %two = binary add i64 %b, %c
    %three = binary add i64 %d, %e
    %four = binary add i64 %three, %f
    %five = binary xor i64 %one, %two
    %six = binary xor i64 %five, %four
    %result = binary add i64 %six, %count
    return i64 %result
}
function @oracle(%x : i64, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^body: %next]
    %value = phi i64 [^entry: %x, ^body: %step]
    %more = cmp ult i64 %i, %n
    branch %more, ^body, ^done
  block ^body:
    %a = binary ushr i64 %value, 1
    %b = binary shl i64 %value, 63
    %step = binary or i64 %a, %b
    %next = binary add i64 %i, 1
    jump ^loop
  block ^done:
    return i64 %value
}
function @main() -> i64 [role=entry] {
  block ^entry:
    jump ^loop
  block ^loop:
    %k = phi i64 [^entry: 0, ^next: %knext]
    %seed = phi i64 [^entry: 81985529216486895, ^next: %newseed]
    %n = binary and i64 %k, 63
    %expected = call i64 @oracle(%seed, %n)
    %actual = call i64 @rotate_right(%seed, %k)
    %live = call i64 @with_live_values(%seed, 11, 22, %k, 33, 44, 55, 66)
    %sum = binary add i64 %expected, 11
    %mix = binary xor i64 %sum, 55
    %mix2 = binary xor i64 %mix, 165
    %expected_live = binary add i64 %mix2, %k
    %bad_a = cmp ne i64 %actual, %expected
    %bad_b = cmp ne i64 %live, %expected_live
    %bad = binary or i64 %bad_a, %bad_b
    branch %bad, ^fail, ^next
  block ^next:
    %product = binary mul i64 %seed, 6364136223846793005
    %newseed = binary add i64 %product, 1
    %knext = binary add i64 %k, 1
    %more = cmp ult i64 %knext, 128
    branch %more, ^loop, ^done
  block ^fail:
    return i64 1
  block ^done:
    return i64 0
}
