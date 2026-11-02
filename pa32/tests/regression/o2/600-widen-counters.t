# An `int` counter that does not wrap reads as a 64-bit counter where it indexes
# memory. What the loop adds to it goes into the base before the loop, a literal
# into the access. What reads the `int` reads an exact truncation.
function @direct(%p : ptr, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    %has = cmp gt i32 %n, 0
    branch %has, ^pre, ^done

  block ^pre:
    jump ^loop

  block ^loop:
    %i = phi i32 [^pre: 0, ^loop: %next]
    %sum = phi i64 [^pre: 0, ^loop: %more]
    %at = index i64 %p, %i
    %v = load typed i64 %at
    %more = binary add i64 %sum, %v
    %next = binary add nowrap i32 %i, 1
    %again = cmp lt i32 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    %r = phi i64 [^entry: 0, ^loop: %more]
    return i64 %r
}

function @offsets(%p : ptr, %q : ptr, %start : i32, %k : i32, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: %start, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %more]
    %a = binary add nowrap i32 %i, %k
    %at_a = index i64 %p, %a
    %va = load typed i64 %at_a
    %b = binary add nowrap i32 %i, 3
    %at_b = index i64 %p, %b
    %vb = load typed i64 %at_b
    %c = binary sub nowrap i32 %i, 1
    %at_c = index i64 %q, %c
    %vc = load typed i64 %at_c
    %d = binary add nowrap i32 %c, %k
    %at_d = index i64 %q, %d
    %vd = load typed i64 %at_d
    %s1 = binary add i64 %va, %vb
    %s2 = binary add i64 %vc, %vd
    %s3 = binary add i64 %s1, %s2
    %more = binary add i64 %sum, %s3
    %next = binary add nowrap i32 %i, 1
    %again = cmp lt i32 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    return i64 %more
}

# The access through the next value reads the phi and a displacement, so the
# step can go after it and the back edge copies nothing.
function @next_value(%p : ptr, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %more]
    %next = binary add nowrap i32 %i, 1
    %at = index i64 %p, %next
    %v = load typed i64 %at
    %here = index i64 %p, %i
    %w = load typed i64 %here
    %d = binary sub i64 %v, %w
    %more = binary add i64 %sum, %d
    %again = cmp lt i32 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    return i64 %more
}

# The base changes in the loop, so the sum is formed there.
function @moving_base(%p : ptr, %k : i32, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %more]
    %base = load typed ptr %p
    %a = binary add nowrap i32 %i, %k
    %at = index i64 %base, %a
    %v = load typed i64 %at
    %more = binary add i64 %sum, %v
    store typed ptr %at, %p
    %next = binary add nowrap i32 %i, 1
    %again = cmp lt i32 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    return i64 %more
}

function @downward(%p : ptr, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: %n, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %more]
    %at = index i64 %p, %i
    %v = load typed i64 %at
    %more = binary add i64 %sum, %v
    %next = binary sub nowrap i32 %i, 1
    %again = cmp ge i32 %next, 0
    branch %again, ^loop, ^done

  block ^done:
    return i64 %more
}

function @unsigned(%p : ptr, %n : u32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi u32 [^entry: 2, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %more]
    %at = index i64 %p, %i
    %v = load typed i64 %at
    %more = binary add i64 %sum, %v
    %next = binary add nowrap u32 %i, 2
    %again = cmp ult u32 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    return i64 %more
}

# What reads the counter as an `int` reads it truncated.
function @also_narrow(%p : ptr, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^loop: %next]
    %sum = phi i64 [^entry: 0, ^loop: %more]
    %at = index i64 %p, %i
    %v = load typed i64 %at
    %d = binary div i32 %i, 3
    %w = convert sext i64 i32 %d
    %s = binary add i64 %v, %w
    %more = binary add i64 %sum, %s
    %next = binary add nowrap i32 %i, 1
    %again = cmp lt i32 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    return i64 %more
}

# The inner loop extends the outer counter before it starts.
function @nested(%p : ptr, %n : i32, %m : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^outer

  block ^outer:
    %i = phi i32 [^entry: 0, ^outer_next: %i2]
    %total = phi i64 [^entry: 0, ^outer_next: %sum2]
    jump ^inner

  block ^inner:
    %j = phi i32 [^outer: %i, ^inner: %j2]
    %sum = phi i64 [^outer: %total, ^inner: %sum2]
    %at = index i64 %p, %j
    %v = load typed i64 %at
    %sum2 = binary add i64 %sum, %v
    %j2 = binary add nowrap i32 %j, 1
    %again = cmp lt i32 %j2, %m
    branch %again, ^inner, ^outer_next

  block ^outer_next:
    %i2 = binary add nowrap i32 %i, 1
    %more = cmp lt i32 %i2, %n
    branch %more, ^outer, ^done

  block ^done:
    return i64 %sum2
}

# A counter that may wrap stays as it is.
function @wrapping(%p : ptr, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i32 [^entry: 0, ^latch: %next]
    %sum = phi i64 [^entry: 0, ^latch: %more]
    %at = index i64 %p, %i
    %v = load typed i64 %at
    %more = binary add i64 %sum, %v
    %odd = cmp eq i64 %v, 0
    branch %odd, ^done, ^latch

  block ^latch:
    %next = binary add i32 %i, 7
    %again = cmp ne i32 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    %r = phi i64 [^loop: 0, ^latch: %more]
    return i64 %r
}
