# A call of the function itself in tail position becomes a jump to the entry.

function @sum_down(%n : i64, %total : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp eq i64 %n, 0
    branch %c, ^done, ^more
  block ^done:
    return i64 %total
  block ^more:
    %m = binary sub i64 %n, 1
    %t = binary add i64 %total, %n
    %r = call i64 @sum_down(%m, %t)
    return i64 %r
}

# The result reaches the return through a phi of a shared block, and the second
# parameter does not change.
function @step(%n : i32, %k : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %c = cmp gt i32 %n, %k
    branch %c, ^high, ^low
  block ^high:
    %m = binary sub i32 %n, %k
    %r = call i32 @step(%m, %k)
    jump ^out
  block ^low:
    jump ^out
  block ^out:
    %v = phi i32 [^high: %r, ^low: %n]
    return i32 %v
}

# A function of no result.
function @walk(%p : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    %c = cmp eq i64 %n, 0
    branch %c, ^done, ^more
  block ^done:
    return void
  block ^more:
    store i64 %n, %p
    %m = binary sub i64 %n, 1
    call void @walk(%p, %m)
    return void
}

# A local the frame keeps to itself may share the one frame.
function @local(%n : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %a = addr $s
    %b = index i8 %a, 8
    store volatile i64 %n, %b
    %v = load volatile i64 %b
    %c = cmp eq i64 %v, 0
    branch %c, ^done, ^more
  block ^done:
    return i64 %v
  block ^more:
    %m = binary sub i64 %v, 1
    %r = call i64 @local(%m)
    return i64 %r
}

declare function @use(%p : ptr) -> void

# The callee's frame must differ from this one when an address leaves it.
function @escaping(%n : i64) -> i64 [no_inline=yes] {
  slot $s : obj<8x8>
  block ^entry:
    %a = addr $s
    store volatile i64 %n, %a
    call void @use(%a)
    %c = cmp eq i64 %n, 0
    branch %c, ^done, ^more
  block ^done:
    return i64 0
  block ^more:
    %m = binary sub i64 %n, 1
    %r = call i64 @escaping(%m)
    return i64 %r
}

# A result used after the call is no tail position.
function @after(%n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp eq i64 %n, 0
    branch %c, ^done, ^more
  block ^done:
    return i64 1
  block ^more:
    %m = binary sub i64 %n, 1
    %r = call i64 @after(%m)
    %s = binary xor i64 %r, %n
    return i64 %s
}
