# A 32-bit divide by a value that is not a constant reads the low halves of
# its operands, directly in memory or from a register; its results are
# canonical for the type. A constant divisor and a wider type keep the
# 64-bit divide, and narrow types are divided in their canonical forms.
function @signed_quotient(%a : i32, %b : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary div i32 %a, %b
    return i32 %r
}
function @signed_remainder(%a : i32, %b : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary mod i32 %a, %b
    return i32 %r
}
function @unsigned_quotient(%a : u32, %b : u32) -> u32 [no_inline=yes] {
  block ^entry:
    %r = binary udiv u32 %a, %b
    return u32 %r
}
function @unsigned_remainder(%a : u32, %b : u32) -> u32 [no_inline=yes] {
  block ^entry:
    %r = binary umod u32 %a, %b
    return u32 %r
}
function @signed_memory(%a : i32, %p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %b = load i32 %p
    %r = binary mod i32 %a, %b
    return i32 %r
}
function @wide(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary div i64 %a, %b
    return i64 %r
}
function @negative_one(%a : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary div i32 %a, -1
    return i32 %r
}
function @narrow(%a : i16, %b : i16) -> i16 [no_inline=yes] {
  block ^entry:
    %r = binary div i16 %a, %b
    return i16 %r
}
function @main() -> i32 [role=entry] {
  slot $divisor : i32
  block ^entry:
    %p = addr $divisor
    store i32 -3, %p
    %a = call i32 @signed_quotient(-2147483647, -7)
    %b = call i32 @signed_remainder(-2147483647, 7)
    %c = call u32 @unsigned_quotient(4294967289, 7)
    %d = call u32 @unsigned_remainder(4294967289, 10)
    %e = call i32 @signed_memory(-11, %p)
    %f = call i64 @wide(-77, 7)
    %g = call i32 @negative_one(-2147483648)
    %h = call i16 @narrow(-32768, 3)
    %bad_a = cmp ne i32 %a, 306783378
    %bad_b = cmp ne i32 %b, -1
    %bad_c = cmp ne u32 %c, 613566755
    %bad_d = cmp ne u32 %d, 9
    %bad_e = cmp ne i32 %e, -2
    %bad_f = cmp ne i64 %f, -11
    %bad_g = cmp ne i32 %g, -2147483648
    %bad_h = cmp ne i16 %h, -10922
    %any_a = binary or i32 %bad_a, %bad_b
    %any_b = binary or i32 %any_a, %bad_c
    %any_c = binary or i32 %any_b, %bad_d
    %any_d = binary or i32 %any_c, %bad_e
    %any_e = binary or i32 %any_d, %bad_f
    %any_f = binary or i32 %any_e, %bad_g
    %any_g = binary or i32 %any_f, %bad_h
    return i32 %any_g
}
