# The high half of the product of two extended words is one multiply
# (`trunc i64 (ushr i128 (mul i128 (zext a), (zext b)), 64)`: the selector
# emits `mul` and reads rdx). A callee of four of them is priced as four
# multiplies, not as four 128-bit products with their extensions and
# shifts, and goes inline at both calls in `@use`.
function @high4(%a : i64, %b : i64, %c : i64, %d : i64, %e : i64) -> i64 [binding=strong] {
  block ^entry:
    %xa = convert zext i128 i64 %a
    %xb = convert zext i128 i64 %b
    %m1 = binary mul i128 %xa, %xb
    %s1 = binary ushr i128 %m1, 64
    %h1 = convert trunc i64 i128 %s1
    %x1 = convert zext i128 i64 %h1
    %xc = convert zext i128 i64 %c
    %m2 = binary mul i128 %x1, %xc
    %s2 = binary ushr i128 %m2, 64
    %h2 = convert trunc i64 i128 %s2
    %x2 = convert zext i128 i64 %h2
    %xd = convert zext i128 i64 %d
    %m3 = binary mul i128 %x2, %xd
    %s3 = binary ushr i128 %m3, 64
    %h3 = convert trunc i64 i128 %s3
    %x3 = convert zext i128 i64 %h3
    %xe = convert zext i128 i64 %e
    %m4 = binary mul i128 %x3, %xe
    %s4 = binary ushr i128 %m4, 64
    %h4 = convert trunc i64 i128 %s4
    return i64 %h4
}

function @use(%a : i64, %b : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %x = call i64 @high4(%a, %b, %b, %b, %b)
    %y = call i64 @high4(%b, %a, %a, %a, %a)
    %r = binary add i64 %x, %y
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %r = call i64 @use(-1, 4294967296)
    %ok = cmp eq i64 %r, 4294967292
    %t = convert trunc i32 i64 %ok
    %code = binary sub i32 1, %t
    return i32 %code
}
