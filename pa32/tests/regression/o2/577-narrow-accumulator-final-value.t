# `result += 3` on a narrow type steps through its promotion. It may wrap, and
# the loop still has a known result: the widening kind does not matter.
function @unsigned_sum() -> u8 [binding=strong] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^body: %next]
    %sum = phi u8 [^entry: 0, ^body: %narrow]
    %more = cmp lt i32 %i, 8000
    branch %more, ^body, ^done

  block ^body:
    %wide = convert zext i32 u8 %sum
    %added = binary add nowrap i32 %wide, 3
    %narrow = convert trunc u8 i32 %added
    %next = binary add nowrap i32 %i, 1
    jump ^loop

  block ^done:
    return u8 %sum
}

function @signed_difference() -> i16 [binding=strong] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^body: %next]
    %sum = phi i16 [^entry: 7, ^body: %narrow]
    %more = cmp lt i32 %i, 40000
    branch %more, ^body, ^done

  block ^body:
    %wide = convert sext i32 i16 %sum
    %lower = binary sub nowrap i32 %wide, 3
    %narrow = convert trunc i16 i32 %lower
    %next = binary add nowrap i32 %i, 1
    jump ^loop

  block ^done:
    return i16 %sum
}
