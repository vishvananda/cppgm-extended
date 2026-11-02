# Two pointers into one array by multiples of its element size reach the same
# fields of different elements: a store to one field leaves the others, whose
# ranges do not meet it modulo that size. The same field may be the same
# object. So may a field past a product that can wrap, which keeps only the
# power of two in its factor, and a range that runs over the stride.
function @forwarded(%p : ptr, %i : i32, %j : i32) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = convert sext i64 i32 %i
    %b = convert sext i64 i32 %j
    %x = binary mul i64 %a, 24
    %y = binary mul i64 %b, 24
    %e = index i8 %p, %x
    %f = index i8 %p, %y
    %mass = index i8 %f, 16
    %speed = index i8 %e, 8
    %m1 = load typed f64 %mass
    store typed f64 %m1, %speed
    %m2 = load typed f64 %mass
    %s = binary add f64 %m1, %m2
    return f64 %s
}

function @same_field(%p : ptr, %i : i32, %j : i32) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = convert sext i64 i32 %i
    %b = convert sext i64 i32 %j
    %x = binary mul i64 %a, 24
    %y = binary mul i64 %b, 24
    %e = index i8 %p, %x
    %f = index i8 %p, %y
    %mass = index i8 %f, 16
    %other = index i8 %e, 16
    %m1 = load typed f64 %mass
    %d = binary mul f64 %m1, 2
    store typed f64 %d, %other
    %m2 = load typed f64 %mass
    %s = binary add f64 %m1, %m2
    return f64 %s
}

function @wrapping(%p : ptr, %i : i64, %j : i64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %x = binary mul i64 %i, 24
    %y = binary mul i64 %j, 24
    %e = index i8 %p, %x
    %f = index i8 %p, %y
    %mass = index i8 %f, 16
    %speed = index i8 %e, 8
    %m1 = load typed f64 %mass
    %d = binary mul f64 %m1, 2
    store typed f64 %d, %speed
    %m2 = load typed f64 %mass
    %s = binary add f64 %m1, %m2
    return f64 %s
}

function @stepped(%p : ptr, %i : i32, %n : i64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = convert sext i64 i32 %i
    %x = binary mul i64 %a, 24
    %e = index i8 %p, %x
    %mass = index i8 %e, 16
    jump ^head

  block ^head:
    %k = phi i64 [^entry: 0, ^head: %next]
    %s = phi f64 [^entry: 0, ^head: %t]
    %f = index i8 %p, %k
    %m = load typed f64 %mass
    store typed f64 %m, %f
    %t = binary add f64 %s, %m
    %next = binary add i64 %k, 24
    %more = cmp ult i64 %next, %n
    branch %more, ^head, ^done

  block ^done:
    return f64 %t
}

global @data : obj<96x8> = { zero 96 }

function @main() -> i32 [role=entry] {
  block ^entry:
    %c = index i8 @data, 16
    store typed f64 3, %c
    %a = call f64 @forwarded(@data, 1, 0)
    %b = call f64 @same_field(@data, 0, 0)
    %e = call f64 @wrapping(@data, 1, 0)
    %f = call f64 @stepped(@data, 1, 48)
    %g = binary add f64 %a, %b
    %h = binary add f64 %e, %f
    %i = binary add f64 %g, %h
    %r = convert fptosi i32 f64 %i
    return i32 %r
}
