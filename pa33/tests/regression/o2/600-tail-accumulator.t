# A self call whose result an add then returns becomes a jump back to the
# entry with an accumulator: zero on entering, the site adds the other
# operand (a parameter's new value is the loop's phi), and every other return
# adds it to its value. A plain tail call in the same function passes the
# accumulator on. A floating add is left alone.
function @sum(%n : i64, %m : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %done = cmp eq i64 %n, 0
    branch %done, ^base, ^step

  block ^base:
    return i64 %m

  block ^step:
    %odd = binary and i64 %n, 1
    %less = binary sub i64 %n, 1
    branch %odd, ^plain, ^adding

  block ^plain:
    %more = binary add i64 %m, 1
    %p = call i64 @sum(%less, %more)
    return i64 %p

  block ^adding:
    %q = call i64 @sum(%less, %m)
    %r = binary add i64 %n, %q
    return i64 %r
}

function @half(%n : i64, %x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %done = cmp eq i64 %n, 0
    branch %done, ^base, ^step

  block ^base:
    return f64 %x

  block ^step:
    %less = binary sub i64 %n, 1
    %q = call f64 @half(%less, %x)
    %r = binary add f64 %q, 0.5
    return f64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %s = call i64 @sum(100, 5)
    %h = call f64 @half(4, 1)
    %hi = convert fptosi i64 f64 %h
    %t = binary add i64 %s, %hi
    %bad = cmp ne i64 %t, 2608
    %r = convert trunc i32 i64 %bad
    return i32 %r
}
