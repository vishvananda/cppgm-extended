# Code after a vector loop may read the loop's sums and counters. The lanes
# know both when they stop: a sum is the sum of its lanes, and a counter is
# where the vector loop leaves it. So the vector loop runs every whole round,
# the last one too, and leaves to the code after the loop when no pass is
# left; a phi there takes each value from the scalar loop or from the lanes.
# A value of the last pass that is neither keeps the scalar loop: the vector
# loop leaves it at least one pass, which gives the value.

# The sum reaches the code after the loop through a phi of the exit block,
# which also takes the path that skips the loop: the phi takes the lanes' sum
# from the vector loop's exit.
function @sum(%x : ptr, %n : i64) -> u32 [no_inline=yes] {
  block ^entry:
    %any = cmp ne i64 %n, 0
    branch %any, ^body, ^done
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi u32 [^entry: 0, ^body: %t]
    %p = index u32 %x, %i
    %v = load typed u32 %p
    %t = binary add u32 %s, %v
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done
  block ^done:
    %r = phi u32 [^entry: 0, ^body: %t]
    return u32 %r
}

# A sum from a nonzero start and the counter, read by the code after the loop
# in a block of its own: a new phi there takes each.
function @sum_and_count(%x : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: 7, ^body: %t]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %t = binary add i64 %s, %v
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done
  block ^done:
    %scaled = binary mul i64 %next, 3
    %r = binary add i64 %t, %scaled
    return i64 %r
}

# A backward loop.
function @backward(%x : ptr, %n : i64) -> u32 [no_inline=yes] {
  block ^entry:
    %any = cmp ne i64 %n, 0
    branch %any, ^body, ^done
  block ^body:
    %i = phi i64 [^entry: %n, ^body: %next]
    %s = phi u32 [^entry: 11, ^body: %t]
    %next = binary sub i64 %i, 1
    %p = index u32 %x, %next
    %v = load typed u32 %p
    %t = binary add u32 %s, %v
    %more = cmp ugt i64 %next, 0
    branch %more, ^body, ^done
  block ^done:
    %r = phi u32 [^entry: 11, ^body: %t]
    return u32 %r
}

# The last pass's product is not a sum: the scalar loop runs the last passes.
function @last(%x : ptr, %n : i64) -> u32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi u32 [^entry: 0, ^body: %t]
    %p = index u32 %x, %i
    %v = load typed u32 %p
    %w = binary mul u32 %v, 5
    %t = binary add u32 %s, %w
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, %n
    branch %more, ^body, ^done
  block ^done:
    %r = binary xor u32 %t, %w
    return u32 %r
}

# Literal bounds over whole rounds (64 passes, 4 lanes): every vector loop
# ends at the bound, so its exit goes straight to the code after the loop,
# and the scalar loop goes.
function @rounds(%x : ptr) -> u32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi u32 [^entry: 5, ^body: %t]
    %p = index u32 %x, %i
    %v = load typed u32 %p
    %t = binary xor u32 %s, %v
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, 64
    branch %more, ^body, ^done
  block ^done:
    return u32 %t
}

# 66 passes are not whole rounds: the vector loop's exit goes straight to
# the scalar loop, which does the last two.
function @not_rounds(%x : ptr) -> u32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi u32 [^entry: 5, ^body: %t]
    %p = index u32 %x, %i
    %v = load typed u32 %p
    %t = binary xor u32 %s, %v
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, 66
    branch %more, ^body, ^done
  block ^done:
    return u32 %t
}

# The inner loop's sum goes on into the outer loop's phi.
function @nested(%x : ptr, %rows : i64) -> u32 [no_inline=yes] {
  block ^entry:
    jump ^outer
  block ^outer:
    %r = phi i64 [^entry: 0, ^latch: %r_next]
    %acc = phi u32 [^entry: 0, ^latch: %mixed]
    %row = binary mul i64 %r, 32
    %base = index u32 %x, %row
    jump ^inner
  block ^inner:
    %c = phi i64 [^outer: 0, ^inner: %c_next]
    %s = phi u32 [^outer: %acc, ^inner: %t]
    %p = index u32 %base, %c
    %v = load typed u32 %p
    %t = binary add u32 %s, %v
    %c_next = binary add i64 %c, 1
    %more = cmp ult i64 %c_next, 32
    branch %more, ^inner, ^latch
  block ^latch:
    %rot = binary shl u32 %t, 3
    %mixed = binary xor u32 %rot, %t
    %r_next = binary add i64 %r, 1
    %again = cmp ult i64 %r_next, %rows
    branch %again, ^outer, ^done
  block ^done:
    return u32 %mixed
}
