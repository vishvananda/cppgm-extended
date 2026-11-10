# Floating parameters stay in registers above -O0: a leaf needs no frame, a
# loop reads no memory, and exchanged parameters take a parallel move. A value
# live across a call goes to the frame, as no XMM register survives one. The
# scratch area remains for what uses it.
function @sub(%a : f64, %b : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = binary sub f64 %a, %b
    return f64 %r
}
function @exchanged(%a : f64, %b : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = call f64 @sub(%b, %a)
    return f64 %r
}
function @across_calls(%a : f64, %b : f64, %c : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %x = call f64 @sub(%a, %b)
    %y = call f64 @sub(%b, %c)
    %m = binary mul f64 %x, %y
    %s = binary add f64 %m, %a
    %t = binary add f64 %s, %c
    return f64 %t
}
function @scaled_sum(%p : ptr, %n : i64, %scale : f64) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^body: %next]
    %total = phi f64 [^entry: 0.0, ^body: %sum]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %at = index f64 [projection=array_element] %p, %i
    %v = load f64 %at
    %product = binary mul f64 %v, %scale
    %sum = binary add f64 %total, %product
    %next = binary add i64 %i, 1
    jump ^head

  block ^done:
    return f64 %total
}
function @mixed(%i : i64, %a : f64, %j : i64, %b : f32, %k : i64) -> f64 [no_inline=yes] {
  block ^entry:
    %wide = convert fpext f64 f32 %b
    %sum = binary add i64 %i, %j
    %all = binary add i64 %sum, %k
    %n = convert sitofp f64 i64 %all
    %m = binary mul f64 %n, %a
    %r = binary add f64 %m, %wide
    return f64 %r
}
function @extended_round_trip(%a : f64) -> i64 [no_inline=yes] {
  block ^entry:
    %e = convert fpext f80 f64 %a
    %i = convert fptosi i64 f80 %e
    return i64 %i
}
function @main() -> i64 [role=entry] {
  slot $values : obj<24x8>
  block ^entry:
    %d = call f64 @exchanged(1.0, 2.0)
    %dok = cmp eq f64 %d, 1.0
    branch %dok, ^across, ^bad1

  block ^bad1:
    return i64 1

  block ^across:
    %a = call f64 @across_calls(1.0, 2.0, 3.0)
    %aok = cmp eq f64 %a, 5.0
    branch %aok, ^loop, ^bad2

  block ^bad2:
    return i64 2

  block ^loop:
    %base = addr $values
    store f64 1.5, %base
    %second = index f64 [projection=array_element] %base, 1
    store f64 2.5, %second
    %third = index f64 [projection=array_element] %base, 2
    store f64 4.0, %third
    %s = call f64 @scaled_sum(%base, 3, 2.0)
    %sok = cmp eq f64 %s, 16.0
    branch %sok, ^mix, ^bad3

  block ^bad3:
    return i64 3

  block ^mix:
    %m = call f64 @mixed(1, 2.0, 2, 0.5, 3)
    %mok = cmp eq f64 %m, 12.5
    branch %mok, ^wide, ^bad4

  block ^bad4:
    return i64 4

  block ^wide:
    %w = call i64 @extended_round_trip(9.0)
    %wok = cmp eq i64 %w, 9
    branch %wok, ^pass, ^bad5

  block ^bad5:
    return i64 5

  block ^pass:
    return i64 0
}
