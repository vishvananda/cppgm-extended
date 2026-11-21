target aarch64 1
# At -O3 for Arm, a broadcast whose one reader is the multiply right after it
# stays beside it (the multiply reads the repeated lane in place, so the
# broadcast is never made); a broadcast other instructions read too is made
# wherever it stays, and leaves the loop. @one_product keeps %k4 beside its
# multiply; in @two_products, two multiplies read %k4 and it moves to the
# preheader.
function @one_product(%a : ptr, %b : ptr, %n : i64) -> i32 {
  block ^entry:
    %k = vector fromi32 269850533
    jump ^loop

  block ^loop:
    %acc = phi v128 [^entry: 0, ^loop: %acc2]
    %i = phi i64 [^entry: 0, ^loop: %i1]
    %q = index inbounds i32 %b, %i
    %u = load v128 %q
    %h = vector psrldi %u, 3
    %l = vector pslldi %u, 5
    %x = vector pxor %h, %l
    %y = vector paddd %x, %u
    %z = vector psrldi %y, 7
    %t = vector pxor %y, %z
    %s = vector pslldi %t, 11
    %w = vector pxor %t, %s
    %k4 = vector pshufd %k, 0
    %m = vector pmulld %w, %k4
    %acc1 = vector paddd %acc, %m
    %acc2 = vector pxor %acc1, %u
    %i1 = binary add nowrap i64 %i, 4
    %more = cmp ult i64 %i1, %n
    branch %more, ^loop, ^done

  block ^done:
    %r = vector toi32 %acc2
    return i32 %r
}

function @two_products(%a : ptr, %b : ptr, %n : i64) -> i32 {
  block ^entry:
    %k = vector fromi32 269850533
    jump ^loop

  block ^loop:
    %acc = phi v128 [^entry: 0, ^loop: %acc2]
    %i = phi i64 [^entry: 0, ^loop: %i1]
    %p = index inbounds i32 %a, %i
    %v = load v128 %p
    %k4 = vector pshufd %k, 0
    %m = vector pmulld %v, %k4
    %h = vector psrldi %v, 3
    %l = vector pslldi %v, 5
    %x = vector pxor %h, %l
    %z = vector psrldi %x, 7
    %u = vector pxor %x, %z
    %m2 = vector pmulld %u, %k4
    %acc1 = vector paddd %acc, %m
    %acc2 = vector paddd %acc1, %m2
    %i1 = binary add nowrap i64 %i, 4
    %more = cmp ult i64 %i1, %n
    branch %more, ^loop, ^done

  block ^done:
    %w = vector toi32 %acc2
    return i32 %w
}
