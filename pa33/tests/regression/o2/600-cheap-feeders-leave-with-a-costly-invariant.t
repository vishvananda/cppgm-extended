# A loop with a call holds its values across the call, so a cheap invariant
# computation stays in it. A costly one (a division, a multiplication) leaves,
# and the cheap computations it needs leave with it: in @quotient the
# subtraction and the conversion feed the division, and all three run before
# the loop. In @offset the invariant sum feeds nothing costly: it stays.
global @seen : f64 = 0

function @note(%v : f64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load f64 @seen
    %new = binary add f64 %old, %v
    store f64 %new, @seen
    return void
}

function @quotient(%row : i64, %half : f64, %height : f64, %n : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %y = convert sitofp f64 i64 %row
    %centered = binary sub f64 %y, %half
    %scaled = binary div f64 %centered, %height
    %x = convert sitofp f64 i64 %i
    %v = binary add f64 %x, %scaled
    call void @note(%v)
    %next = binary add nowrap i64 %i, 1
    %again = cmp lt i64 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    return void
}

function @offset(%a : f64, %b : f64, %n : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %base = binary add f64 %a, %b
    %x = convert sitofp f64 i64 %i
    %v = binary add f64 %x, %base
    call void @note(%v)
    %next = binary add nowrap i64 %i, 1
    %again = cmp lt i64 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    call void @quotient(6, 2, 4, 3)
    call void @offset(1, 2, 2)
    %seen = load f64 @seen
    %t = convert fptosi i32 f64 %seen
    return i32 %t
}
