# A strict test of a floating value against zero orders the value and its
# negation alike (`x < 0` exactly when `x < -x`, for the zeros and an
# unordered `x` too), so a choice between the two is their lane `max` or
# `min`: `x < 0 ? -x : x` is `max(-x, x)`, which gives `x` for a zero or an
# unordered `x` as the branch does. The arm's negation runs on both paths.
# `@not_strict` tells the zeros apart and stays a branch. A magnitude is
# never below zero: `@below` loses the test that asks.
function @magnitude(%x : f32) -> f32 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp lt f32 %x, 0f
    branch %c, ^negate, ^out
  block ^negate:
    %n = unary neg f32 %x
    jump ^out
  block ^out:
    %r = phi f32 [^negate: %n, ^entry: %x]
    return f32 %r
}

function @positive_or_negated(%x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp gt f64 %x, 0
    branch %c, ^out, ^negate
  block ^negate:
    %n = unary neg f64 %x
    jump ^out
  block ^out:
    %r = phi f64 [^negate: %n, ^entry: %x]
    return f64 %r
}

function @negative_magnitude(%x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp lt f64 %x, 0
    branch %c, ^out, ^negate
  block ^negate:
    %n = unary neg f64 %x
    jump ^out
  block ^out:
    %r = phi f64 [^negate: %n, ^entry: %x]
    return f64 %r
}

function @zero_first(%x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp gt f64 0, %x
    branch %c, ^negate, ^out
  block ^negate:
    %n = unary neg f64 %x
    jump ^out
  block ^out:
    %r = phi f64 [^negate: %n, ^entry: %x]
    return f64 %r
}

function @not_strict(%x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp le f64 %x, 0
    branch %c, ^negate, ^out
  block ^negate:
    %n = unary neg f64 %x
    jump ^out
  block ^out:
    %r = phi f64 [^negate: %n, ^entry: %x]
    return f64 %r
}

function @below(%x : f64, %p : ptr) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %n = unary neg f64 %x
    %a = vector fromf64 %n
    %b = vector fromf64 %x
    %l = vector maxpd %a, %b
    %m = vector tof64 %l
    %c = cmp lt f64 %m, 0
    branch %c, ^yes, ^no
  block ^yes:
    store i32 1, %p
    return i32 1
  block ^no:
    return i32 0
}

function @main() -> i32 [role=entry] {
  slot $seen : i32
  block ^entry:
    %s = addr $seen
    store i32 0, %s
    %j = call i32 @below(-2, %s)
    %k = load i32 %s
    %a = call f32 @magnitude(-3f)
    %b = call f32 @magnitude(2f)
    %c = call f64 @positive_or_negated(-5)
    %d = call f64 @positive_or_negated(4)
    %e = call f64 @negative_magnitude(-6)
    %f = call f64 @negative_magnitude(7)
    %g = call f64 @zero_first(-8)
    %h = call f64 @zero_first(9)
    %i = call f64 @not_strict(-1)
    %ok0 = cmp eq f32 %a, 3f
    %ok1 = cmp eq f32 %b, 2f
    %ok2 = cmp eq f64 %c, 5
    %ok3 = cmp eq f64 %d, 4
    %ok4 = cmp eq f64 %e, -6
    %ok5 = cmp eq f64 %f, -7
    %ok6 = cmp eq f64 %g, 8
    %ok7 = cmp eq f64 %h, 9
    %ok8 = cmp eq f64 %i, 1
    %ok9 = cmp eq i32 %j, 0
    %ok10 = cmp eq i32 %k, 0
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %a2 = binary and i64 %a1, %ok3
    %a3 = binary and i64 %a2, %ok4
    %a4 = binary and i64 %a3, %ok5
    %a5 = binary and i64 %a4, %ok6
    %a6 = binary and i64 %a5, %ok7
    %a7 = binary and i64 %a6, %ok8
    %a8 = binary and i64 %a7, %ok9
    %a9 = binary and i64 %a8, %ok10
    %r = convert trunc i32 i64 %a9
    %code = binary sub i32 1, %r
    return i32 %code
}
