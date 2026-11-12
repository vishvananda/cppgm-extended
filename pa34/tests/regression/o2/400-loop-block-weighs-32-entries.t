# A loop's exit takes one part in 32 of its test's executions, so a block of
# the loop weighs 32 entries. %l is read once in the loop and %s sixteen
# times after it: %l is the heavier value, and takes its callee-saved
# register before %s (the registers go out in one order, heaviest value
# first). The counter and its bound, read by the loop's test, come before
# both.
global @seen : i64 = 0

function @step(%v : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return i64 %new
}

function @walk(%n : i64, %s : i64, %l : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^body: %next]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %x = binary add i64 %i, %l
    %r = call i64 @step(%x)
    %next = binary add i64 %i, 1
    jump ^head

  block ^done:
    %t0 = load i64 @seen
    %t1 = binary add i64 %t0, %s
    %t2 = binary xor i64 %t1, %s
    %t3 = binary add i64 %t2, %s
    %t4 = binary xor i64 %t3, %s
    %t5 = binary add i64 %t4, %s
    %t6 = binary xor i64 %t5, %s
    %t7 = binary add i64 %t6, %s
    %t8 = binary xor i64 %t7, %s
    %t9 = binary add i64 %t8, %s
    %t10 = binary xor i64 %t9, %s
    %t11 = binary add i64 %t10, %s
    %t12 = binary xor i64 %t11, %s
    %t13 = binary add i64 %t12, %s
    %t14 = binary xor i64 %t13, %s
    %t15 = binary add i64 %t14, %s
    %t16 = binary xor i64 %t15, %s
    return i64 %t16
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %r = call i64 @walk(5, 3, 7)
    %m = binary umod i64 %r, 251
    %t = convert trunc i32 i64 %m
    return i32 %t
}
