# The sum lives across a call on a rare path, so only a split lets it keep a
# register. It picks that register before the rest of its phi web, and the phis
# after the call take the same one: the hot path moves no floating value.
function @cold(%a : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = binary add f64 %a, %a
    return f64 %r
}

function @sum(%p : ptr, %n : i64, %limit : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %has = cmp gt i64 %n, 0
    branch %has, ^loop, ^empty

  block ^empty:
    return f64 0

  block ^loop:
    %i = phi i64 [^entry: 0, ^next: %i2]
    %s = phi f64 [^entry: 0, ^next: %s3]
    %at = index f64 %p, %i
    %v = load typed f64 %at
    %over = cmp gt f64 %v, %limit
    branch %over, ^skip, ^use

  block ^use:
    %neg = cmp lt f64 %v, 0
    branch %neg, ^slow, ^add

  block ^slow:
    %c = call f64 @cold(%v)
    jump ^add

  block ^add:
    %w = phi f64 [^use: %v, ^slow: %c]
    %m = binary mul f64 %w, %v
    %s2 = binary add f64 %s, %m
    jump ^next

  block ^skip:
    jump ^next

  block ^next:
    %s3 = phi f64 [^add: %s2, ^skip: %s]
    %i2 = binary add i64 %i, 1
    %more = cmp lt i64 %i2, %n
    branch %more, ^loop, ^end

  block ^end:
    return f64 %s3
}

function @main() -> i32 [role=entry] {
  slot $values : obj<24x8>
  block ^entry:
    %a = addr $values
    store typed f64 2, %a
    %b = index f64 %a, 1
    store typed f64 -1, %b
    %c = index f64 %a, 2
    store typed f64 30, %c
    %r = call f64 @sum(%a, 3, 10)
    %t = convert fptosi i32 f64 %r
    return i32 %t
}
