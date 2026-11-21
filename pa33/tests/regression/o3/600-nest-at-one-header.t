# At -O3, an outer loop whose body begins with a do-while loop, so that the
# inner back edge and the outer one both lead to ^body, is two loops: the
# inner back edge's cycle holds no other latch, and the outer back edge and
# the entry move to a new header in front of ^body whose phis merge what
# they bring. The outer loop's values are then invariant in the inner loop:
# @scaled_rows computes its row factor and offset once per row, and
# @row_xor's inner loop of 64 words is unrolled into vector reads. The rule
# needs a value that the inner back edge brings back unchanged and the outer
# one steps (%r), and an inner back edge taken by a counter's test (%j1
# against 16) in a loop that leaves only into the outer one: @skip_steps,
# whose two back edges step every value (the index by one past an even word,
# by two past an odd one), @lower_bound, whose back edge that halves the
# range keeps %first but tests the data (a binary search), and @gapped_sum,
# which walks one range in segments and stores each segment's running sum
# (its first back edge keeps %end, but ^head, where the walk leaves, would be
# the inner loop's), are each one loop and keep one header.
function @scaled_rows(%a : ptr, %rows : i32, %factor : i32) -> i32 {
  block ^entry:
    %enter = cmp gt i32 %rows, 0
    branch %enter, ^body, ^done

  block ^body:
    %r = phi i32 [^entry: 0, ^body: %r, ^next: %r1]
    %total = phi i32 [^entry: 0, ^body: %total, ^next: %sum]
    %j = phi i32 [^entry: 0, ^body: %j1, ^next: 0]
    %s = phi i32 [^entry: 0, ^body: %s1, ^next: 0]
    %scale = binary mul i32 %r, %factor
    %row = binary mul i32 %r, 16
    %k = binary add i32 %row, %j
    %p = index i32 %a, %k
    %v = load i32 %p
    %w = binary add i32 %v, %scale
    %s1 = binary xor i32 %s, %w
    %j1 = binary add nowrap i32 %j, 1
    %more = cmp lt i32 %j1, 16
    branch %more, ^body, ^next

  block ^next:
    %sum = binary add i32 %total, %s1
    %r1 = binary add nowrap i32 %r, 1
    %again = cmp lt i32 %r1, %rows
    branch %again, ^body, ^done

  block ^done:
    %result = phi i32 [^entry: 0, ^next: %sum]
    return i32 %result
}

function @row_xor(%a : ptr, %rows : i32) -> u32 {
  block ^entry:
    %enter = cmp gt i32 %rows, 0
    branch %enter, ^body, ^done

  block ^body:
    %r = phi i32 [^entry: 0, ^body: %r, ^next: %r1]
    %total = phi u32 [^entry: 0, ^body: %total, ^next: %sum]
    %j = phi i32 [^entry: 0, ^body: %j1, ^next: 0]
    %x = phi u32 [^entry: 2166136261, ^body: %x1, ^next: 2166136261]
    %row = binary mul nowrap i32 %r, 64
    %k = binary add nowrap i32 %row, %j
    %p = index u32 %a, %k
    %v = load u32 %p
    %x1 = binary xor u32 %x, %v
    %j1 = binary add nowrap i32 %j, 1
    %more = cmp lt i32 %j1, 64
    branch %more, ^body, ^next

  block ^next:
    %mixed = binary mul u32 %total, 16777619
    %sum = binary add u32 %mixed, %x1
    %r1 = binary add nowrap i32 %r, 1
    %again = cmp lt i32 %r1, %rows
    branch %again, ^body, ^done

  block ^done:
    %result = phi u32 [^entry: 0, ^next: %sum]
    return u32 %result
}

function @skip_steps(%a : ptr, %n : i32) -> i32 {
  block ^entry:
    %enter = cmp gt i32 %n, 0
    branch %enter, ^head, ^done

  block ^head:
    %i = phi i32 [^entry: 0, ^test: %i1, ^wide: %i2]
    %s = phi i32 [^entry: 0, ^test: %s1, ^wide: %s1]
    %p = index i32 %a, %i
    %v = load i32 %p
    %s1 = binary add i32 %s, %v
    %i1 = binary add nowrap i32 %i, 1
    %more = cmp lt i32 %i1, %n
    branch %more, ^test, ^done

  block ^test:
    %odd = binary and i32 %v, 1
    %even = cmp eq i32 %odd, 0
    branch %even, ^head, ^wide

  block ^wide:
    %i2 = binary add nowrap i32 %i1, 1
    %again = cmp lt i32 %i2, %n
    branch %again, ^head, ^done

  block ^done:
    %result = phi i32 [^entry: 0, ^head: %s1, ^wide: %s1]
    return i32 %result
}

function @lower_bound(%a : ptr, %n : i64, %x : i32) -> i64 {
  block ^entry:
    jump ^head

  block ^head:
    %first = phi i64 [^entry: 0, ^probe: %first, ^right: %first1]
    %len = phi i64 [^entry: %n, ^probe: %half, ^right: %len1]
    %more = cmp gt i64 %len, 0
    branch %more, ^probe, ^done

  block ^probe:
    %half = binary shr i64 %len, 1
    %mid = binary add i64 %first, %half
    %p = index i32 %a, %mid
    %v = load i32 %p
    %below = cmp lt i32 %v, %x
    branch %below, ^right, ^head

  block ^right:
    %first1 = binary add i64 %mid, 1
    %rest = binary sub i64 %len, %half
    %len1 = binary sub i64 %rest, 1
    jump ^head

  block ^done:
    return i64 %first
}

function @gapped_sum(%a : ptr, %sums : ptr, %n : i64, %w : i64, %gap : i64) -> i32 {
  block ^entry:
    jump ^head

  block ^head:
    %p = phi i64 [^entry: 0, ^word: %p1, ^skip: %p2]
    %end = phi i64 [^entry: %w, ^word: %end, ^skip: %end1]
    %r = phi i64 [^entry: 0, ^word: %r, ^skip: %r1]
    %s = phi i32 [^entry: 0, ^word: %s1, ^skip: %s1]
    %go = cmp lt i64 %p, %n
    branch %go, ^word, ^done

  block ^word:
    %q = index i32 %a, %p
    %v = load i32 %q
    %s1 = binary add i32 %s, %v
    %p1 = binary add i64 %p, 1
    %at = cmp eq i64 %p1, %end
    branch %at, ^skip, ^head

  block ^skip:
    %o = index i32 %sums, %r
    store i32 %s1, %o
    %r1 = binary add i64 %r, 1
    %p2 = binary add i64 %p1, %gap
    %end1 = binary add i64 %p2, %w
    jump ^head

  block ^done:
    return i32 %s
}
