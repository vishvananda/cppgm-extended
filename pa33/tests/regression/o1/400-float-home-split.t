# The two floating values live across a loop whose only call is on a path taken
# when a value is not a number. They stay in registers on the hot path; only
# the call's path saves and restores them.
function @slow(%a : f64, %b : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = binary add f64 %a, %b
    return f64 %r
}

function @scaled_sum(%p : ptr, %n : i64, %x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %has = cmp gt i64 %n, 0
    branch %has, ^loop, ^done

  block ^loop:
    %i = phi i64 [^entry: 0, ^next: %i2]
    %s = phi f64 [^entry: 0, ^next: %s2]
    %at = index f64 %p, %i
    %v = load typed f64 %at
    %m = binary mul f64 %v, %x
    %n2 = binary add f64 %m, %y
    %nan = cmp ne f64 %n2, %n2
    branch %nan, ^cold, ^next

  block ^cold:
    %c = call f64 @slow(%v, %x)
    jump ^next

  block ^next:
    %t = phi f64 [^loop: %n2, ^cold: %c]
    %s2 = binary add f64 %s, %t
    %i2 = binary add nowrap i64 %i, 1
    %more = cmp lt i64 %i2, %n
    branch %more, ^loop, ^done

  block ^done:
    %r = phi f64 [^entry: 0, ^next: %s2]
    return f64 %r
}

function @main() -> i32 [role=entry] {
  slot $values : obj<16x8>

  block ^entry:
    %p = addr $values
    store typed f64 1, %p
    %q = index i8 %p, 8
    store typed f64 2, %q
    %r = call f64 @scaled_sum(%p, 2, 2, 1)
    %v = convert fptosi i32 f64 %r
    return i32 %v
}
