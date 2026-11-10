# A count that steps on a compare's carry flag steps with adc or sbb: its chain
# is one instruction long, against two for lea and cmov. A condition the flag
# does not decide keeps the select.
global @values = {
  i32 3
  i32 9
  i32 1
  i32 7
}

function @count_below(%p : ptr, %n : i64, %limit : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %j]
    %c = phi i64 [^entry: 0, ^loop: %next]
    %a = index i32 %p, %i
    %v = load typed i32 %a
    %w = convert zext i64 i32 %v
    %up = binary add i64 %c, 1
    %below = cmp ult i64 %w, %limit
    %next = select i64 %below, %up, %c
    %j = binary add i64 %i, 1
    %more = cmp ult i64 %j, %n
    branch %more, ^loop, ^done
  block ^done:
    return i64 %next
}

function @count_at_least(%a : i64, %b : i64, %c : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %up = binary add i64 %c, 1
    %ge = cmp uge i64 %a, %b
    %r = select i64 %ge, %up, %c
    return i64 %r
}

function @down_below(%a : i64, %b : i64, %c : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %down = binary sub i64 %c, 1
    %lt = cmp ult i64 %a, %b
    %r = select i64 %lt, %down, %c
    return i64 %r
}

function @down_at_least(%a : i64, %b : i64, %c : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %down = binary sub i64 %c, 1
    %lt = cmp ult i64 %a, %b
    %r = select i64 %lt, %c, %down
    return i64 %r
}

function @count_float(%a : f64, %b : f64, %c : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %up = binary add i32 %c, 1
    %ge = cmp ge f64 %a, %b
    %r = select i32 %ge, %up, %c
    return i32 %r
}

function @count_signed(%a : i64, %b : i64, %c : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %up = binary add i64 %c, 1
    %lt = cmp lt i64 %a, %b
    %r = select i64 %lt, %up, %c
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @count_below(@values, 4, 8)
    %b = call i64 @count_at_least(5, 5, 10)
    %c = call i64 @down_below(5, 6, 10)
    %d = call i64 @down_at_least(6, 5, 10)
    %e = call i32 @count_float(2, 1, 100)
    %f = call i32 @count_float(1, 2, 100)
    %g = call i64 @count_signed(-1, 0, 7)
    %h = call i64 @count_at_least(5, 6, 10)
    %s = binary add i64 %a, %b
    %t = binary add i64 %c, %d
    %u = binary add i64 %g, %h
    %v = binary add i64 %s, %t
    %w = binary add i64 %u, %v
    %x = convert trunc i32 i64 %w
    %y = binary add i32 %e, %f
    %z = binary add i32 %x, %y
    return i32 %z
}
