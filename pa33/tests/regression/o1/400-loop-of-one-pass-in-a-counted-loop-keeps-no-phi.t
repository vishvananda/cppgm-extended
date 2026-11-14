# A loop of one pass inside a counted loop of two passes. The inner loop
# becomes straight code, and its header keeps no phi of one entry: the
# header's values are the entering ones. The outer loop's passes are then
# copied with those values. `@nest` stores `j + 5` at element `j` for `j` of 0
# and 1. Each outer pass adds the stored value and the inner counter's final
# value, 1, to the sum: the result is 13.
function @nest(%p : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^outer

  block ^outer:
    %j = phi i64 [^entry: 0, ^outer_next: %j1]
    %sum = phi i64 [^entry: 0, ^outer_next: %sum1]
    %more = cmp le i64 %j, 1
    branch %more, ^inner_entry, ^done

  block ^inner_entry:
    jump ^inner

  block ^inner:
    %k = phi i64 [^inner_entry: 0, ^inner_next: %k1]
    %acc = phi i64 [^inner_entry: %sum, ^inner_next: %acc1]
    %again = cmp le i64 %k, 0
    branch %again, ^body, ^outer_next

  block ^body:
    %scaled = binary mul i64 %k, 8
    %at = binary add i64 %j, %scaled
    %cell = index i64 %p, %at
    %value = binary add i64 %j, 5
    store i64 %value, %cell
    %acc1 = binary add i64 %acc, %value
    jump ^inner_next

  block ^inner_next:
    %k1 = binary add i64 %k, 1
    jump ^inner

  block ^outer_next:
    %sum1 = binary add i64 %acc, %k
    %j1 = binary add i64 %j, 1
    jump ^outer

  block ^done:
    return i64 %sum
}

function @main() -> i32 [role=entry] {
  slot $a : obj<16x8>

  block ^entry:
    %p = addr $a
    store i64 0, %p
    %q = index i64 %p, 1
    store i64 0, %q
    %r = call i64 @nest(%p)
    %first = load i64 %p
    %second = load i64 %q
    %ok0 = cmp eq i64 %r, 13
    %ok1 = cmp eq i64 %first, 5
    %ok2 = cmp eq i64 %second, 6
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %t = convert trunc i32 i64 %a1
    %code = binary sub i32 1, %t
    return i32 %code
}
