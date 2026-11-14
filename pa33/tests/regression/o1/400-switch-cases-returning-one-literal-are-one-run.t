# The cases of a switch that each return the same literal from a block of
# their own (`case A: return true; case B: return true;`) reach the same
# code: over consecutive values the switch is one range test, and the two
# returns that are left are the test's result. `@two_results` keeps its
# switch: its cases return different literals.
function @in_run(%k : u16) -> u8 [binding=strong, no_inline=yes] {
  block ^entry:
    switch %k, ^no, 420:^c0, 421:^c1, 422:^c2, 423:^c3, 424:^c4, 425:^c5

  block ^c0:
    return u8 1

  block ^c1:
    return u8 1

  block ^c2:
    return u8 1

  block ^c3:
    return u8 1

  block ^c4:
    return u8 1

  block ^c5:
    return u8 1

  block ^no:
    return u8 0
}

function @two_results(%k : u16) -> u8 [binding=strong, no_inline=yes] {
  block ^entry:
    switch %k, ^no, 420:^c0, 421:^c1, 422:^c2, 423:^c3

  block ^c0:
    return u8 1

  block ^c1:
    return u8 2

  block ^c2:
    return u8 1

  block ^c3:
    return u8 2

  block ^no:
    return u8 0
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call u8 @in_run(419)
    %b = call u8 @in_run(420)
    %c = call u8 @in_run(425)
    %d = call u8 @in_run(426)
    %e = call u8 @two_results(421)
    %f = call u8 @two_results(422)
    %g = call u8 @two_results(7)
    %ok0 = cmp eq u8 %a, 0
    %ok1 = cmp eq u8 %b, 1
    %ok2 = cmp eq u8 %c, 1
    %ok3 = cmp eq u8 %d, 0
    %ok4 = cmp eq u8 %e, 2
    %ok5 = cmp eq u8 %f, 1
    %ok6 = cmp eq u8 %g, 0
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %a2 = binary and i64 %a1, %ok3
    %a3 = binary and i64 %a2, %ok4
    %a4 = binary and i64 %a3, %ok5
    %a5 = binary and i64 %a4, %ok6
    %r = convert trunc i32 i64 %a5
    %code = binary sub i32 1, %r
    return i32 %code
}
