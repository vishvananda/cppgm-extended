# A widened counter still has its exact narrow value at the loop test.
# Exercise both the empty path and repeated entries to the counted body.
global @input : i64 [binding=internal] = 3

function @count(%limit : i32) -> i64 [binding=internal, no_inline=yes, unwind=no] {
  block ^entry:
    %positive = cmp gt i32 %limit, 0
    branch %positive, ^loop, ^empty

  block ^loop:
    %counter = phi i64 [^entry: 0, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %total]
    %value = load volatile i64 @input
    %total = binary add i64 %sum, %value
    %next = binary add nowrap i64 %counter, 1
    %narrow = convert trunc exact i32 i64 %next
    %again = cmp lt i32 %narrow, %limit
    branch %again, ^loop, ^done

  block ^empty:
    return i64 0

  block ^done:
    return i64 %total
}

function @main() -> i32 [role=entry, unwind=no] {
  block ^entry:
    %empty = call i64 @count(0)
    %negative = call i64 @count(-3)
    %short = call i64 @count(7)
    %long = call i64 @count(31)
    %empty_bad = cmp ne i64 %empty, 0
    %negative_bad = cmp ne i64 %negative, 0
    %short_bad = cmp ne i64 %short, 21
    %long_bad = cmp ne i64 %long, 93
    %a = binary or u8 %empty_bad, %negative_bad
    %b = binary or u8 %short_bad, %long_bad
    %bad = binary or u8 %a, %b
    %status = convert zext i32 u8 %bad
    return i32 %status
}
