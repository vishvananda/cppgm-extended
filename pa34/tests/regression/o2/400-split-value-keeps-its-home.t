# Nine values live through a loop whose head calls a function: three keep
# callee-saved registers, three stay in their frame homes, and three are
# split: a caller-saved register in the latch, the home in the head. Each
# split value is stored once before the loop and reloaded on the edge into
# the latch. Its home never changes, so the edge back to the head stores
# nothing: the latch ends with the counter's step and the jump.
global @seen : i64 = 0

function @step(%v : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return i64 %new
}

function @walk(%a : i64, %b : i64, %c : i64, %d : i64, %e : i64, %f : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %g = binary mul i64 %a, %b
    %h = binary mul i64 %c, %d
    %k = binary mul i64 %e, %f
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^latch: %i2]
    %s = phi i64 [^entry: 0, ^latch: %s9]
    %r = call i64 @step(%i)
    %go = cmp lt i64 %r, 1000
    branch %go, ^latch, ^done

  block ^latch:
    %s1 = binary add i64 %s, %a
    %s2 = binary xor i64 %s1, %b
    %s3 = binary add i64 %s2, %c
    %s4 = binary xor i64 %s3, %d
    %s5 = binary add i64 %s4, %e
    %s6 = binary xor i64 %s5, %f
    %s7 = binary add i64 %s6, %g
    %s8 = binary xor i64 %s7, %h
    %t1 = binary add i64 %s8, %k
    %t2 = binary xor i64 %t1, %a
    %t3 = binary add i64 %t2, %b
    %t4 = binary xor i64 %t3, %c
    %t5 = binary add i64 %t4, %d
    %t6 = binary xor i64 %t5, %e
    %t7 = binary add i64 %t6, %f
    %t8 = binary xor i64 %t7, %g
    %t9 = binary add i64 %t8, %h
    %s9 = binary xor i64 %t9, %k
    %i2 = binary add i64 %i, 7
    jump ^head

  block ^done:
    return i64 %s
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %r = call i64 @walk(1, 2, 3, 4, 5, 6)
    %t = convert trunc i32 i64 %r
    %u = binary and i32 %t, 127
    return i32 %u
}
