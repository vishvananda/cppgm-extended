# Two 32-bit values packed into 64 bits and read back are the values, and each
# integer retyped through the other signedness and back is itself.
function @parts(%a : i32, %b : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %low = convert zext i64 i32 %a
    %wide = convert zext i64 i32 %b
    %high = binary shl i64 %wide, 32
    %pair = binary or i64 %low, %high
    %first = convert trunc i32 i64 %pair
    %shifted = binary ushr i64 %pair, 32
    %second = convert trunc i32 i64 %shifted
    %r = binary sub i32 %first, %second
    return i32 %r
}

function @retyped(%a : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %u = copy u32 %a
    %s = copy i32 %u
    return i32 %s
}

# The value in the high half may have more than 32 bits: the shift keeps them.
function @not_packed(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %high = binary shl i64 %b, 32
    %pair = binary or i64 %a, %high
    %shifted = binary ushr i64 %pair, 32
    return i64 %shifted
}
