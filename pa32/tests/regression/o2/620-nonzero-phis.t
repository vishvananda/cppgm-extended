# A phi whose every input is a nonzero literal, a value tested on its edge, or
# a phi of the same kind is not zero, so a branch on it goes. A recursion
# inlined into itself tests the value each of its entries tested. One input
# nothing tested keeps the test.
function @sum(%m : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    branch %m, ^head, ^zero

  block ^head:
    %i = phi i64 [^entry: %m, ^latch: %j]
    %s = phi i64 [^entry: 0, ^latch: %t]
    branch %i, ^latch, ^never

  block ^latch:
    %t = binary add i64 %s, %i
    %j = binary sub i64 %i, 1
    branch %j, ^head, ^done

  block ^never:
    return i64 -1

  block ^zero:
    return i64 0

  block ^done:
    return i64 %t
}

function @compared(%p : ptr, %q : ptr, %n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %p0 = cmp ne ptr %p, 0
    branch %p0, ^other, ^zero

  block ^other:
    %q0 = cmp eq ptr %q, 0
    branch %q0, ^zero, ^pick

  block ^pick:
    branch %n, ^use_q, ^merge

  block ^use_q:
    jump ^merge

  block ^merge:
    %x = phi ptr [^pick: %p, ^use_q: %q]
    %c = cmp eq ptr %x, 0
    branch %c, ^never, ^fine

  block ^never:
    return i64 -1

  block ^fine:
    return i64 1

  block ^zero:
    return i64 0
}

function @untested(%a : i64, %b : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    branch %a, ^left, ^merge

  block ^left:
    jump ^merge

  block ^merge:
    %x = phi i64 [^entry: %b, ^left: %a]
    branch %x, ^one, ^two

  block ^one:
    return i64 1

  block ^two:
    return i64 2
}

global @cell : i64 = 0

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @sum(3)
    %b = call i64 @sum(0)
    %c = call i64 @compared(@cell, @cell, 1)
    %d = call i64 @compared(@cell, 0, 1)
    %e = call i64 @untested(0, 0)
    %f = call i64 @untested(1, 0)
    %g = binary add i64 %a, %b
    %h = binary add i64 %c, %d
    %i = binary add i64 %e, %f
    %j = binary add i64 %g, %h
    %k = binary add i64 %i, %j
    %r = convert trunc i32 i64 %k
    return i32 %r
}
