# A phi whose every input is the same few integer steps with literals on what
# another phi of its block takes from that predecessor is those steps on the
# other phi: @behind's parent is its hole less three on both edges, and
# @behind_half's parent is half of one less than its hole. In @apart the two
# edges take different steps, and both phis stay.
function @behind(%n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %h0 = binary sub i64 %n, 1
    %p0 = binary add i64 %n, -4
    jump ^head

  block ^head:
    %parent = phi i64 [^entry: %p0, ^body: %next]
    %hole = phi i64 [^entry: %h0, ^body: %parent]
    %sum = phi i64 [^entry: 0, ^body: %sum2]
    %more = cmp gt i64 %hole, 0
    branch %more, ^body, ^done

  block ^body:
    %sum2 = binary add i64 %sum, %parent
    %next = binary sub i64 %parent, 3
    jump ^head

  block ^done:
    return i64 %sum
}

function @behind_half(%n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %h0 = binary sub i64 %n, 1
    %p0a = binary add i64 %n, -2
    %p0 = binary udiv i64 %p0a, 2
    jump ^head

  block ^head:
    %parent = phi i64 [^entry: %p0, ^body: %next]
    %hole = phi i64 [^entry: %h0, ^body: %parent]
    %sum = phi i64 [^entry: 0, ^body: %sum2]
    %more = cmp ne i64 %hole, 0
    branch %more, ^body, ^done

  block ^body:
    %sum2 = binary add i64 %sum, %parent
    %p1 = binary sub i64 %parent, 1
    %next = binary udiv i64 %p1, 2
    jump ^head

  block ^done:
    return i64 %sum
}

function @apart(%n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %h0 = binary sub i64 %n, 1
    %p0 = binary add i64 %n, -3
    jump ^head

  block ^head:
    %parent = phi i64 [^entry: %p0, ^body: %next]
    %hole = phi i64 [^entry: %h0, ^body: %parent]
    %sum = phi i64 [^entry: 0, ^body: %sum2]
    %more = cmp gt i64 %hole, 0
    branch %more, ^body, ^done

  block ^body:
    %sum2 = binary add i64 %sum, %parent
    %next = binary sub i64 %parent, 3
    jump ^head

  block ^done:
    return i64 %sum
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %a = call i64 @behind(10)
    %a_bad = cmp ne i64 %a, 9
    %b = call i64 @behind_half(20)
    %b_bad = cmp ne i64 %b, 14
    %c = call i64 @apart(10)
    %c_bad = cmp ne i64 %c, 10
    %bad0 = binary or i64 %a_bad, %b_bad
    %bad = binary or i64 %bad0, %c_bad
    return i64 %bad
}
