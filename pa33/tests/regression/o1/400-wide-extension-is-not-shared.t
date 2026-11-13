# An extension to i128 is not a value to share: a backend keeps a shared
# i128 in memory, while a multiply reads its own extension from the register
# it extends. `@twice` keeps one extension of `%x` per product; the equal
# 64-bit sums of `@narrow` are still one value.
function @twice(%x : i64, %a : i64, %b : i64) -> i64 {
  block ^entry:
    %w1 = convert zext i128 i64 %x
    %wa = convert zext i128 i64 %a
    %p1 = binary mul i128 %wa, %w1
    %h1 = binary ushr i128 %p1, 64
    %r1 = convert trunc i64 i128 %h1
    %w2 = convert zext i128 i64 %x
    %wb = convert zext i128 i64 %b
    %p2 = binary mul i128 %wb, %w2
    %h2 = binary ushr i128 %p2, 64
    %r2 = convert trunc i64 i128 %h2
    %s = binary add i64 %r1, %r2
    return i64 %s
}
function @narrow(%x : i64, %y : i64) -> i64 {
  block ^entry:
    %a = binary add i64 %x, %y
    %b = binary add i64 %x, %y
    %c = binary mul i64 %a, %b
    return i64 %c
}
