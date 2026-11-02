# The second block repeats the compare of its only predecessor: it reads the
# flags that compare left. @not_every_edge has another way in: it keeps its own.

function @three_way(%a : i32, %b : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %c = cmp gt i32 %a, %b
    branch %c, ^above, ^rest
  block ^above:
    return i32 1
  block ^rest:
    %d = cmp lt i32 %a, %b
    branch %d, ^below, ^same
  block ^below:
    return i32 2
  block ^same:
    return i32 3
}

function @not_every_edge(%a : i32, %b : i32, %e : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %c = cmp gt i32 %a, %b
    branch %c, ^above, ^mid
  block ^mid:
    %f = cmp eq i32 %e, 0
    branch %f, ^rest, ^above
  block ^above:
    %x = phi i32 [^entry: 1, ^mid: 4]
    return i32 %x
  block ^rest:
    %d = cmp lt i32 %a, %b
    branch %d, ^below, ^same
  block ^below:
    return i32 2
  block ^same:
    return i32 3
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @three_way(1, 2)
    %b = call i32 @three_way(2, 1)
    %c = call i32 @three_way(2, 2)
    %d = call i32 @not_every_edge(1, 2, 0)
    %e = call i32 @not_every_edge(2, 2, 0)
    %f = call i32 @not_every_edge(2, 2, 1)
    %g = call i32 @not_every_edge(3, 2, 1)
    %x = binary mul i32 %a, 10
    %y = binary add i32 %x, %b
    %z = binary mul i32 %y, 10
    %w = binary add i32 %z, %c
    %s = binary add i32 %d, %e
    %t = binary add i32 %s, %f
    %u = binary add i32 %t, %g
    %r = binary add i32 %w, %u
    %q = binary umod i32 %r, 200
    return i32 %q
}
