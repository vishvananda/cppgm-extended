# A test of a value against a literal that only skips a block is not needed
# where the block, computed from the literal, gives the value the skip does.
# `@untagged` is a tagged pointer's read after its null test: with `%v` zero
# the block's masks and select give zero, the value of the skipped path, so
# the block runs on both paths and the test goes. `@word` tests `!= 5` the
# other way round (5 & 4 selects 0). `@differs` keeps its test: with `%v`
# zero its block gives 1, not the 2 of the skipped path. `@returned` has no
# merge: its block returns the value and the skipped path returns the literal.
function @untagged(%v : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %null = cmp eq i64 %v, 0
    branch %null, ^done, ^read

  block ^read:
    %tag = binary and i64 %v, 4
    %ptr = binary and i64 %v, -8
    %r = select i64 %tag, 0, %ptr
    jump ^done

  block ^done:
    %out = phi i64 [^read: %r, ^entry: 0]
    return i64 %out
}

function @word(%v : i64, %w : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %other = cmp ne i64 %v, 5
    branch %other, ^read, ^done

  block ^read:
    %tag = binary and i64 %v, 4
    %sum = binary add i64 %v, %w
    %r = select i64 %tag, 0, %sum
    jump ^done

  block ^done:
    %out = phi i64 [^read: %r, ^entry: 0]
    return i64 %out
}

function @differs(%v : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %null = cmp eq i64 %v, 0
    branch %null, ^done, ^read

  block ^read:
    %ptr = binary and i64 %v, -8
    %a = binary xor i64 %ptr, 1
    %b = binary mul i64 %a, %a
    jump ^done

  block ^done:
    %out = phi i64 [^read: %b, ^entry: 2]
    return i64 %out
}

function @returned(%v : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %null = cmp eq i64 %v, 0
    branch %null, ^none, ^read

  block ^read:
    %tag = binary and i64 %v, 4
    %ptr = binary and i64 %v, -8
    %r = select i64 %tag, 0, %ptr
    return i64 %r

  block ^none:
    return i64 0
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @untagged(0)
    %b = call i64 @untagged(4096)
    %c = call i64 @untagged(4100)
    %d = call i64 @word(5, 9)
    %e = call i64 @word(8, 9)
    %f = call i64 @word(4, 9)
    %g = call i64 @differs(0)
    %h = call i64 @differs(16)
    %ok0 = cmp eq i64 %a, 0
    %ok1 = cmp eq i64 %b, 4096
    %ok2 = cmp eq i64 %c, 0
    %ok3 = cmp eq i64 %d, 0
    %ok4 = cmp eq i64 %e, 17
    %ok5 = cmp eq i64 %f, 0
    %ok6 = cmp eq i64 %g, 2
    %ok7 = cmp eq i64 %h, 289
    %i = call i64 @returned(0)
    %j = call i64 @returned(4096)
    %k = call i64 @returned(4100)
    %ok8 = cmp eq i64 %i, 0
    %ok9 = cmp eq i64 %j, 4096
    %ok10 = cmp eq i64 %k, 0
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %a2 = binary and i64 %a1, %ok3
    %a3 = binary and i64 %a2, %ok4
    %a4 = binary and i64 %a3, %ok5
    %a5 = binary and i64 %a4, %ok6
    %a6 = binary and i64 %a5, %ok7
    %a7 = binary and i64 %a6, %ok8
    %a8 = binary and i64 %a7, %ok9
    %a9 = binary and i64 %a8, %ok10
    %r = convert trunc i32 i64 %a9
    %code = binary sub i32 1, %r
    return i32 %code
}
