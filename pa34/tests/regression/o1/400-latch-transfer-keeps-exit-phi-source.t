# A loop block's transfers run at its test (see
# 400-latch-transfers-run-at-the-test) only where the other successor reads
# no register they write. Here the exit's phi takes the loop's phi %at and
# shares its register, so the exit edge holds no move that reads it: the
# transfer of %to into that register stays on the back edge, and @last
# returns the last index it visited (4), not the 0 that ended the walk.
global @next : obj<40x8> [binding=internal] = {
  i64 0
  i64 3
  i64 0
  i64 4
  i64 0
}

function @last(%start : i64, %skip : i64) -> i64 [binding=internal, no_inline=yes] {
  block ^entry:
    branch %skip, ^done, ^walk

  block ^walk:
    %at = phi i64 [^entry: %start, ^walk: %to]
    %slot = index i64 @next, %at
    %to = load i64 %slot
    %end = cmp eq i64 %to, 0
    branch %end, ^done, ^walk

  block ^done:
    %r = phi i64 [^entry: 99, ^walk: %at]
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @last(1, 0)
    %b = call i64 @last(2, 1)
    %sum = binary add i64 %a, %b
    %t = convert trunc i32 i64 %sum
    return i32 %t
}
