# Splitting i128 values into halves keeps a multiplication whole: its
# operands are put together again. An operand that was an extension of an
# i64 is that extension again, a high half of 0 a zext and a high half of
# the low half's sign a sext, not an or of two shifted halves: the back end
# reads such a product's high half as one multiply. @mixed has one of each.
function @signed(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %x = convert sext i128 i64 %a
    %y = convert sext i128 i64 %b
    %p = binary mul i128 %x, %y
    %h = binary shr i128 %p, 64
    %r = convert trunc i64 i128 %h
    return i64 %r
}

function @mixed(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %x = convert zext i128 i64 %a
    %y = convert sext i128 i64 %b
    %p = binary mul i128 %x, %y
    %h = binary ushr i128 %p, 64
    %r = convert trunc i64 i128 %h
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %s = call i64 @signed(-1, 6)
    %m = call i64 @mixed(-1, -1)
    %bad_s = cmp ne i64 %s, -1
    %bad_m = cmp ne i64 %m, -2
    %a = convert zext i64 i1 %bad_s
    %b = convert zext i64 i1 %bad_m
    %ab = binary or i64 %a, %b
    %status = convert trunc i32 i64 %ab
    return i32 %status
}
