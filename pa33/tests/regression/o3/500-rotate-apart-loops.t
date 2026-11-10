# @shared_exit: the second loop's exit phi reads the first loop's header phi on
# the edge from the second header. Rotating the second copies that input for
# its latch, so the first waits a round: both rotate, and the input stays read.
function @shared_exit(%n : i64, %m : i64, %p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^first

  block ^first:
    %a = phi i64 [^entry: 0, ^first_body: %a_next]
    %more = cmp lt i64 %a, %n
    branch %more, ^first_body, ^after

  block ^first_body:
    %at = index i64 %p, %a
    store i64 %a, %at
    %a_next = binary add i64 %a, 1
    jump ^first

  block ^after:
    %skip = cmp eq i64 %m, 0
    branch %skip, ^join, ^mid

  block ^mid:
    jump ^second

  block ^second:
    %b = phi i64 [^mid: 0, ^second_body: %b_next]
    %again = cmp lt i64 %b, %m
    branch %again, ^second_body, ^join

  block ^second_body:
    %to = index i64 %p, %b
    store i64 %m, %to
    %b_next = binary add i64 %b, 1
    jump ^second

  block ^join:
    %r = phi i64 [^after: 1, ^second: %a]
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
