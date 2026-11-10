# Pure values move to the block nearest their readers: the chain %sum, %twice
# into the arm that reads it, the step %next to the test at the end of the loop
# body. %outer stays before the loop that reads it, and %shared stays above the
# two arms that both read it.
global @cell : i64 = 0

function @chain(%a : i64, %b : i64, %flag : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %sum = binary add i64 %a, %b
    %twice = binary mul i64 %sum, 3
    %shared = binary xor i64 %a, 5
    branch %flag, ^read, ^skip

  block ^read:
    %left = binary add i64 %twice, %shared
    return i64 %left

  block ^skip:
    %right = binary sub i64 %shared, 1
    return i64 %right
}

function @steps(%n : i64, %k : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %outer = binary mul i64 %k, 7
    jump ^head

  block ^head:
    %i = phi i64 [^entry: %n, ^latch: %next]
    %t = phi i64 [^entry: 0, ^latch: %u]
    %next = binary sub i64 %i, 1
    %odd = binary and i64 %i, 1
    branch %odd, ^add, ^latch

  block ^add:
    %v = load i64 @cell
    %w = binary add i64 %v, %outer
    store i64 %w, @cell
    jump ^latch

  block ^latch:
    %u = phi i64 [^head: %t, ^add: %i]
    branch %next, ^head, ^done

  block ^done:
    %r = load i64 @cell
    %out = binary add i64 %r, %u
    return i64 %out
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @chain(2, 3, 1)
    %b = call i64 @chain(2, 3, 0)
    %c = call i64 @steps(4, 1)
    %x = binary add i64 %a, %b
    %y = binary add i64 %x, %c
    %z = convert trunc i32 i64 %y
    return i32 %z
}
