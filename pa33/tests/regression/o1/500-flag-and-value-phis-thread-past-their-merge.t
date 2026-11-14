# A merge holds a value phi and a flag phi (an inlined callee's result and
# whether it holds one) and branches on the flag. Each predecessor's flag
# decides the side, so the predecessor jumps there. The value phi is read
# past the merge only by the successors' phis, along the merge's own edges:
# a moved predecessor's input takes its place there.
# In @walk the merge ends the loop's pass: the blocks that produce a value
# jump back to the header, and the one that produces none leaves the loop.
# In @pair the two sides meet again: the producing blocks jump to the join.
global @seen : i64 = 0

function @note(%v : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return void
}

function @walk(%start : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %at = phi i64 [^entry: %start, ^merge: %next]
    %low = binary and i64 %at, 3
    %two = cmp eq i64 %low, 2
    branch %two, ^halve, ^other

  block ^halve:
    %half = binary ushr i64 %at, 1
    jump ^merge

  block ^other:
    %none = cmp eq i64 %low, 0
    branch %none, ^stop, ^drop

  block ^drop:
    %less = binary sub i64 %at, 1
    jump ^merge

  block ^stop:
    jump ^merge

  block ^merge:
    %next = phi i64 [^halve: %half, ^drop: %less, ^stop: 0]
    %more = phi i64 [^halve: 1, ^drop: 1, ^stop: 0]
    branch %more, ^head, ^done

  block ^done:
    return i64 %at
}

function @pair(%x : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %odd = binary and i64 %x, 1
    branch %odd, ^first, ^even

  block ^first:
    %up = binary add i64 %x, 5
    jump ^merge

  block ^even:
    %big = cmp gt i64 %x, 100
    branch %big, ^second, ^merge

  block ^second:
    %down = binary sub i64 %x, 100
    jump ^merge

  block ^merge:
    %value = phi i64 [^first: %up, ^second: %down, ^even: 0]
    %holds = phi i64 [^first: 1, ^second: 1, ^even: 0]
    branch %holds, ^join, ^missing

  block ^missing:
    call void @note(9)
    jump ^join

  block ^join:
    %result = phi i64 [^merge: %value, ^missing: 7]
    return i64 %result
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @walk(27)
    %b = call i64 @walk(6)
    %c = call i64 @pair(3)
    %d = call i64 @pair(250)
    %e = call i64 @pair(4)
    %s1 = binary add i64 %a, %b
    %s2 = binary add i64 %s1, %c
    %s3 = binary add i64 %s2, %d
    %s4 = binary add i64 %s3, %e
    %seen = load i64 @seen
    %s5 = binary add i64 %s4, %seen
    %r = binary umod i64 %s5, 251
    %t = convert trunc i32 i64 %r
    return i32 %t
}
