# A merge of two values with work on both arms keeps its branch: the mask
# refills and the pointer steps one time in eight, which predicts, and two
# selects would run both arms on every path. With work on one arm only, or
# one value, the merge still becomes selects.
function @both(%mask : i64, %p : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp ugt i64 %mask, 1
    branch %c, ^shift, ^refill

  block ^shift:
    %next = binary ushr i64 %mask, 1
    jump ^merge

  block ^refill:
    %step = index i8 %p, 1
    jump ^merge

  block ^merge:
    %m = phi i64 [^shift: %next, ^refill: 128]
    %q = phi ptr [^shift: %p, ^refill: %step]
    %d = binary sub ptr %q, %p
    %r = binary add i64 %m, %d
    return i64 %r
}

function @one_arm(%mask : i64, %p : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp ugt i64 %mask, 1
    branch %c, ^shift, ^merge

  block ^shift:
    %next = binary ushr i64 %mask, 1
    %step = index i8 %p, 1
    jump ^merge

  block ^merge:
    %m = phi i64 [^shift: %next, ^entry: 128]
    %q = phi ptr [^shift: %step, ^entry: %p]
    %d = binary sub ptr %q, %p
    %r = binary add i64 %m, %d
    return i64 %r
}

function @one_value(%mask : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp ugt i64 %mask, 1
    branch %c, ^shift, ^refill

  block ^shift:
    %next = binary ushr i64 %mask, 1
    jump ^merge

  block ^refill:
    %other = binary add i64 %mask, 128
    jump ^merge

  block ^merge:
    %m = phi i64 [^shift: %next, ^refill: %other]
    return i64 %m
}

global @cell : i64 = 0

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @both(4, @cell)
    %b = call i64 @both(1, @cell)
    %c = call i64 @one_arm(4, @cell)
    %d = call i64 @one_arm(1, @cell)
    %e = call i64 @one_value(6)
    %f = call i64 @one_value(1)
    %g = binary add i64 %a, %b
    %h = binary add i64 %c, %d
    %i = binary add i64 %e, %f
    %j = binary add i64 %g, %h
    %k = binary add i64 %i, %j
    %r = convert trunc i32 i64 %k
    return i32 %r
}
