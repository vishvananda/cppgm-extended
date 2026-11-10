# A negation and the arms of a select that only a return reads read their
# operands' low bits, so neither parameter is extended; a compare's Boolean
# returned as a byte has no extension either, and each caller extends what it
# reads wide.
function @negate(%a : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = unary neg i32 %a
    return i32 %r
}

function @larger(%a : i32, %b : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp gt i32 %a, %b
    %r = select i32 %c, %a, %b
    return i32 %r
}

function @below(%a : i32, %b : i32) -> u8 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp lt i32 %a, %b
    %r = convert trunc u8 i64 %c
    return u8 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %n = call i32 @negate(5)
    %wide = convert sext i64 i32 %n
    %minus = cmp eq i64 %wide, -5
    branch %minus, ^next, ^bad

  block ^next:
    %m = call i32 @larger(-7, 3)
    %mw = convert sext i64 i32 %m
    %three = cmp eq i64 %mw, 3
    branch %three, ^last, ^bad

  block ^last:
    %b = call u8 @below(-7, 3)
    %bw = convert zext i64 u8 %b
    %one = cmp eq i64 %bw, 1
    branch %one, ^good, ^bad

  block ^good:
    return i32 0

  block ^bad:
    return i32 1
}
