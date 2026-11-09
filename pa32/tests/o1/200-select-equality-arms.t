# An equality edge can use its compared value in place of an equal literal.
# Floating equality and width-changing comparisons do not prove equal bits.
function @equal_literal(%x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %same = cmp eq i64 %x, 37
    %r = select i64 %same, 37, %y
    return i64 %r
}
function @different_literal(%x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %different = cmp ne i64 %x, 37
    %r = select i64 %different, %y, 37
    return i64 %r
}
function @truth_zero(%x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %r = select i64 %x, %y, 0
    return i64 %r
}
function @pointer_null(%p : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    %null = cmp eq ptr %p, nullptr
    %adjusted = index i8 %p, -8
    %r = select ptr %null, nullptr, %adjusted
    return ptr %r
}
function @unequal_literal(%x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %same = cmp eq i64 %x, 37
    %r = select i64 %same, 38, %y
    return i64 %r
}
function @float_zero(%x : f64) -> i64 [no_inline=yes] {
  block ^entry:
    %bits = copy i64 %x
    %same = cmp eq f64 %x, 0.0
    %r = select i64 %same, 0, %bits
    return i64 %r
}
function @main() -> i64 [role=entry] {
  block ^entry:
    %a = call i64 @equal_literal(37, 91)
    %b = call i64 @equal_literal(38, 91)
    %c = call i64 @different_literal(37, 91)
    %d = call i64 @different_literal(38, 91)
    %e = call i64 @truth_zero(0, 91)
    %f = call i64 @truth_zero(17, 91)
    %g = call ptr @pointer_null(nullptr)
    %h = call i64 @unequal_literal(37, 91)
    %i = call i64 @float_zero(-0.0)
    %a_bad = cmp ne i64 %a, 37
    %b_bad = cmp ne i64 %b, 91
    %c_bad = cmp ne i64 %c, 37
    %d_bad = cmp ne i64 %d, 91
    %e_bad = cmp ne i64 %e, 0
    %f_bad = cmp ne i64 %f, 91
    %g_bad = cmp ne ptr %g, nullptr
    %h_bad = cmp ne i64 %h, 38
    %i_bad = cmp ne i64 %i, 0
    %ab = binary or i1 %a_bad, %b_bad
    %cd = binary or i1 %c_bad, %d_bad
    %ef = binary or i1 %e_bad, %f_bad
    %gh = binary or i1 %g_bad, %h_bad
    %abcd = binary or i1 %ab, %cd
    %efgh = binary or i1 %ef, %gh
    %abcdefgh = binary or i1 %abcd, %efgh
    %bad = binary or i1 %abcdefgh, %i_bad
    return i64 %bad
}
