# A branch inside an innermost loop on a value the loop does not compute is
# made once before the loop; each copy of the loop keeps one path.

declare function @touch(%v : i64) -> void [linkage=c, binding=strong, object=touch, unwind=no]
declare function @thrower(%v : i64) -> void [linkage=c, binding=strong, object=thrower]

# On a parameter. The sum is read past the loop, on the edge of each copy.
function @on_parameter(%flag : i64, %p : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header

  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %sum = phi i64 [^entry: 0, ^latch: %next_sum]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^exit

  block ^body:
    %at = index i64 %p, %i
    %v = load i64 %at
    branch %flag, ^left, ^right

  block ^left:
    %twice = binary add i64 %v, %v
    store i64 %twice, %at
    jump ^latch

  block ^right:
    %neg = unary neg i64 %v
    store i64 %neg, %at
    jump ^latch

  block ^latch:
    %add = phi i64 [^left: %twice, ^right: %neg]
    %next_sum = binary add i64 %sum, %add
    %next = binary add i64 %i, 1
    jump ^header

  block ^exit:
    return i64 %sum
}

# On a compare of the outer counter: the test moves to the outer loop's body.
function @on_outer_compare(%p : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^outer

  block ^outer:
    %i = phi i64 [^entry: 0, ^outer_latch: %i_next]
    %more = cmp lt i64 %i, %n
    branch %more, ^inner_entry, ^exit

  block ^inner_entry:
    jump ^inner

  block ^inner:
    %j = phi i64 [^inner_entry: 0, ^inner_latch: %j_next]
    %first = cmp eq i64 %i, 0
    branch %first, ^seed, ^copy

  block ^seed:
    %at_seed = index i64 %p, %j
    store i64 %j, %at_seed
    jump ^inner_latch

  block ^copy:
    %at_from = index i64 %p, %j
    %v = load i64 %at_from
    %at_i = index i64 %p, %i
    %at_to = index i64 %at_i, %j
    store i64 %v, %at_to
    jump ^inner_latch

  block ^inner_latch:
    %j_next = binary add i64 %j, 1
    %inner_more = cmp lt i64 %j_next, 8
    branch %inner_more, ^inner, ^outer_latch

  block ^outer_latch:
    %i_next = binary add i64 %i, 1
    jump ^outer

  block ^exit:
    return void
}

# A value of the body is read past the loop, in a block the loop does not
# reach directly: it goes through a slot, promoted again.
function @read_past_exit(%flag : i64, %p : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body

  block ^body:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %at = index i64 %p, %i
    %v = load i64 %at
    branch %flag, ^left, ^right

  block ^left:
    call void @touch(%v)
    jump ^latch

  block ^right:
    call void @touch(1)
    jump ^latch

  block ^latch:
    %next = binary add i64 %i, 1
    %more = cmp lt i64 %next, %n
    branch %more, ^body, ^after

  block ^after:
    call void @touch(2)
    jump ^last

  block ^last:
    %r = binary add i64 %v, %next
    return i64 %r
}

# One side leaves the loop: that branch runs at most once as it stands.
function @keep_exit_test(%flag : i64, %p : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header

  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^exit

  block ^body:
    branch %flag, ^latch, ^exit

  block ^latch:
    %at = index i64 %p, %i
    store i64 %i, %at
    %next = binary add i64 %i, 1
    jump ^header

  block ^exit:
    return i64 %i
}

# The test changes on each pass.
function @keep_variant_test(%p : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^header

  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^exit

  block ^body:
    %at = index i64 %p, %i
    %v = load i64 %at
    %odd = binary and i64 %v, 1
    branch %odd, ^left, ^latch

  block ^left:
    store i64 0, %at
    jump ^latch

  block ^latch:
    %next = binary add i64 %i, 1
    jump ^header

  block ^exit:
    return void
}

# A call that may throw: the loop stays.
function @keep_unwinding_call(%flag : i64, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^header

  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^exit

  block ^body:
    branch %flag, ^left, ^latch

  block ^left:
    call void @thrower(%i)
    jump ^latch

  block ^latch:
    %next = binary add i64 %i, 1
    jump ^header

  block ^exit:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
