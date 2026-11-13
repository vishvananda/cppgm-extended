# The first loop's header leaves straight into the second loop's header. The
# second rotates first and stores its phis' values from the first header in a
# block of that edge's own, so the first header still only tests: the first
# loop rotates in the next round, and both loops then run in lanes.
function @fill_both(%n : i64, %m : i64, %p : ptr, %q : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^first

  block ^first:
    %a = phi i64 [^entry: 0, ^first_body: %a_next]
    %more = cmp lt i64 %a, %n
    branch %more, ^first_body, ^second

  block ^first_body:
    %at = index i64 %p, %a
    store i64 %a, %at
    %a_next = binary add i64 %a, 1
    jump ^first

  block ^second:
    %b = phi i64 [^first: 0, ^second_body: %b_next]
    %again = cmp lt i64 %b, %m
    branch %again, ^second_body, ^done

  block ^second_body:
    %to = index i64 %q, %b
    store i64 %m, %to
    %b_next = binary add i64 %b, 1
    jump ^second

  block ^done:
    return void
}

# The second loop goes on from the first loop's count: the edge's block stores
# the first header's phi, and the first loop's rotation then reads it there.
function @go_on(%n : i64, %m : i64, %p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^first

  block ^first:
    %a = phi i64 [^entry: 0, ^first_body: %a_next]
    %more = cmp lt i64 %a, %n
    branch %more, ^first_body, ^second

  block ^first_body:
    %at = index i64 %p, %a
    store i64 %a, %at
    %a_next = binary add i64 %a, 1
    jump ^first

  block ^second:
    %b = phi i64 [^first: %a, ^second_body: %b_next]
    %again = cmp lt i64 %b, %m
    branch %again, ^second_body, ^done

  block ^second_body:
    %to = index i64 %p, %b
    store i64 %m, %to
    %b_next = binary add i64 %b, 1
    jump ^second

  block ^done:
    return i64 %b
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
