# The representation of a floating value with only its sign bit cleared is its
# magnitude, cleared in the XMM register. NaN payloads are kept, and a mask
# that is read again or is not the whole sign complement stays integer work.
global @negative_nan : i64 = -2251799813685247
global @negative_nan32 : i32 = -4194303
function @clobber() -> void [no_inline=yes] {
  block ^entry:
    return void
}
function @magnitude64(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %bits = copy i64 %x
    %masked = binary and i64 %bits, 9223372036854775807
    %result = copy f64 %masked
    return f64 %result
}
function @magnitude32(%x : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %bits = copy i32 %x
    %masked = binary and i32 %bits, 2147483647
    %result = copy f32 %masked
    return f32 %result
}
function @mask_first(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %bits = copy i64 %x
    %masked = binary and i64 9223372036854775807, %bits
    %result = copy f64 %masked
    return f64 %result
}
function @from_memory(%p : ptr) -> f64 [no_inline=yes] {
  block ^entry:
    %x = load f64 %p
    %bits = copy i64 %x
    %masked = binary and i64 %bits, 9223372036854775807
    %result = copy f64 %masked
    return f64 %result
}
function @source_lives_on(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %bits = copy i64 %x
    %masked = binary and i64 %bits, 9223372036854775807
    %magnitude = copy f64 %masked
    %sign = binary div f64 %x, %magnitude
    %result = binary add f64 %sign, %x
    return f64 %result
}
function @across_call(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %bits = copy i64 %x
    call void @clobber()
    %masked = binary and i64 %bits, 9223372036854775807
    %result = copy f64 %masked
    return f64 %result
}
function @mask_read_again(%x : f64) -> i64 [no_inline=yes] {
  block ^entry:
    %bits = copy i64 %x
    %masked = binary and i64 %bits, 9223372036854775807
    %magnitude = copy f64 %masked
    %half = binary mul f64 %magnitude, 0.5
    %halved = copy i64 %half
    %result = binary sub i64 %masked, %halved
    return i64 %result
}
function @other_mask(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %bits = copy i64 %x
    %masked = binary and i64 %bits, 9223372036854775806
    %result = copy f64 %masked
    return f64 %result
}
function @integer_source(%x : i64) -> f64 [no_inline=yes] {
  block ^entry:
    %masked = binary and i64 %x, 9223372036854775807
    %result = copy f64 %masked
    return f64 %result
}
function @narrower_source(%x : f32) -> f64 [no_inline=yes] {
  block ^entry:
    %y = binary add f32 %x, 1.0
    %bits = copy i64 %y
    %masked = binary and i64 %bits, 9223372036854775807
    %result = copy f64 %masked
    return f64 %result
}
function @main() -> i64 [role=entry] {
  slot $cell : f64
  block ^entry:
    %nan = addr @negative_nan
    %negative_nan = load f64 %nan
    %a = call f64 @magnitude64(%negative_nan)
    %a_bits = copy i64 %a
    %bad_a = cmp ne i64 %a_bits, 9221120237041090561
    %b = call f64 @magnitude64(-0.0)
    %b_bits = copy i64 %b
    %bad_b = cmp ne i64 %b_bits, 0
    %nan32 = addr @negative_nan32
    %negative_nan32 = load f32 %nan32
    %c = call f32 @magnitude32(%negative_nan32)
    %c_bits = copy i32 %c
    %bad_c = cmp ne i32 %c_bits, 2143289345
    %d = call f64 @mask_first(-2.5)
    %bad_d = cmp ne f64 %d, 2.5
    %cell = addr $cell
    store f64 -7.5, %cell
    %e = call f64 @from_memory(%cell)
    %bad_e = cmp ne f64 %e, 7.5
    %f = call f64 @source_lives_on(-8.0)
    %bad_f = cmp ne f64 %f, -9.0
    %g = call f64 @across_call(-3.25)
    %bad_g = cmp ne f64 %g, 3.25
    %h = call i64 @mask_read_again(-1.0)
    %bad_h = cmp ne i64 %h, 4503599627370496
    %i = call f64 @other_mask(-1.0000000000000002)
    %i_bits = copy i64 %i
    %bad_i = cmp ne i64 %i_bits, 4607182418800017408
    %j = call f64 @integer_source(-4616189618054758400)
    %bad_j = cmp ne f64 %j, 1.0
    %k = call f64 @narrower_source(0.5)
    %k_bits = copy i64 %k
    %bad_k = cmp ne i64 %k_bits, 1069547520
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %ef = binary or i64 %bad_e, %bad_f
    %gh = binary or i64 %bad_g, %bad_h
    %ij = binary or i64 %bad_i, %bad_j
    %first = binary or i64 %ab, %cd
    %second = binary or i64 %ef, %gh
    %third = binary or i64 %first, %second
    %fourth = binary or i64 %third, %ij
    %failed = binary or i64 %fourth, %bad_k
    %status = convert trunc i32 i64 %failed
    return i64 %status
}
