# An i128 returned from two blocks of an inlined callee: its two words meet
# in phis at the continuation, and their merge is the call's result.
function @pick(%c : i64, %a : i64, %b : i64) -> i128 {
  block ^entry:
    branch %c, ^low, ^high

  block ^low:
    %x = convert zext i128 i64 %a
    return i128 %x

  block ^high:
    %y = convert zext i128 i64 %b
    %z = binary shl i128 %y, 64
    return i128 %z
}

function @words(%c : i64, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %r = call i128 @pick(%c, %a, %b)
    %h = binary ushr i128 %r, 64
    %high = convert trunc i64 i128 %h
    %low = convert trunc i64 i128 %r
    %scaled = binary mul i64 %high, 10
    %sum = binary add i64 %scaled, %low
    return i64 %sum
}

function @main() -> i32 {
  block ^entry:
    %low = call i64 @words(1, 5, 9)
    %high = call i64 @words(0, 5, 9)
    %low_ok = cmp eq i64 %low, 5
    %high_ok = cmp eq i64 %high, 90
    %low_bit = select i32 %low_ok, 0, 1
    %high_bit = select i32 %high_ok, 0, 2
    %status = binary or i32 %low_bit, %high_bit
    return i32 %status
}
