# A location a loop stores on every pass lives in a register through the
# loop: loaded before it and stored on each exit.

global @total : obj<8x8> = { zero 8 }
global @other : obj<8x8> = { zero 8 }
global @pair : obj<16x8> = { zero 16 }

function @opaque(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary add i64 %x, 1
    return i64 %r
}

function @read(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %f = index i8 %p, 8
    %v = load i64 %f
    return i64 %v
}

function @reduction(%n : i64, %p : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %at = index i64 %p, %i
    %v = load typed i32 %at
    %x = convert sext i64 i32 %v
    %t = load typed i64 @total
    %u = binary add i64 %t, %x
    store typed i64 %u, @total
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# Two locations, and two exits: each exit stores both.
function @two_exits(%n : i64, %p : ptr, %f : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %at = index i32 %p, %i
    %v = load typed i32 %at
    %x = convert sext i64 i32 %v
    %t = load typed i64 @total
    %u = binary add i64 %t, %x
    store typed i64 %u, @total
    %s = load typed i64 @other
    %w = binary xor i64 %s, %x
    store typed i64 %w, @other
    %e = cmp eq i64 %x, %f
    branch %e, ^found, ^tail
  block ^tail:
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^found:
    return i64 1
  block ^done:
    return i64 0
}

# Through a pointer the loop does not change, past stores of another type.
function @through_pointer(%n : i64, %s : ptr, %q : ptr, %v : f64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %at = index f64 %q, %i
    store typed f64 %v, %at
    %f = index i8 %s, 8
    %t = load typed i64 %f
    %u = binary add i64 %t, 1
    store typed i64 %u, %f
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# Out of an inner loop and then out of the one around it.
function @nested(%n : i64, %m : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^outer
  block ^outer:
    %i = phi i64 [^entry: 0, ^latch: %i_next]
    jump ^inner
  block ^inner:
    %j = phi i64 [^outer: 0, ^inner: %j_next]
    %t = load typed i64 @total
    %u = binary add i64 %t, %j
    store typed i64 %u, @total
    %j_next = binary add i64 %j, 1
    %d = cmp ult i64 %j_next, %m
    branch %d, ^inner, ^latch
  block ^latch:
    %i_next = binary add i64 %i, 1
    %c = cmp ult i64 %i_next, %n
    branch %c, ^outer, ^done
  block ^done:
    return void
}

# A slot whose address leaves the function after the loop.
function @escaped_slot(%n : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %f = index i8 %p, 8
    store i64 %n, %f
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %t = load i64 %f
    %u = binary add i64 %t, %i
    store i64 %u, %f
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    %r = call i64 @read(%p)
    return i64 %r
}

# A store on only some passes stays: the exit would write memory a run with
# no such pass leaves alone.
function @some_passes(%n : i64, %f : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %e = cmp eq i64 %i, %f
    branch %e, ^write, ^tail
  block ^write:
    store typed i64 %i, @total
    jump ^tail
  block ^tail:
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# The loop may leave before its store.
function @exit_before_store(%n : i64, %f : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %e = cmp eq i64 %i, %f
    branch %e, ^done, ^tail
  block ^tail:
    store typed i64 %i, @total
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# A call might read it.
function @call_in_loop(%n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    store typed i64 %i, @total
    %next = call i64 @opaque(%i)
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# Volatile accesses stay in order.
function @volatile_in_loop(%n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    store typed i64 %i, @total
    %v = load volatile i64 @other
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# A pass may go back to the header without the store: a run that spins there
# never wrote the location.
function @back_without_store(%n : i64, %f : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next, ^write: %next]
    %next = binary add i64 %i, 1
    %e = cmp eq i64 %i, %f
    branch %e, ^head, ^write
  block ^write:
    store typed i64 %i, @total
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# The loop moves the pointer.
function @pointer_walk(%p : ptr, %e : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %at = phi ptr [^entry: %p, ^head: %next]
    store typed i64 1, %at
    %next = index i64 %at, 1
    %c = cmp ne ptr %next, %e
    branch %c, ^head, ^done
  block ^done:
    return void
}

# A pointer the loop steps may reach the location: a store of one type.
function @may_overlap_store(%n : i64, %s : ptr, %q : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %at = index i64 %q, %i
    store typed i64 %i, %at
    store typed i64 %i, %s
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# Likewise a load.
function @may_overlap_load(%n : i64, %s : ptr, %q : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %r = phi i64 [^entry: 0, ^head: %t]
    %at = index i64 %q, %i
    %v = load typed i64 %at
    %t = binary add i64 %r, %v
    store typed i64 %i, %s
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return i64 %t
}

# Another width of the same bytes.
function @other_width(%n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %a = load i32 @pair
    %x = convert sext i64 i32 %a
    %u = binary add i64 %x, %i
    store i64 %u, @pair
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# The neighbouring field of one object does not get in the way.
function @neighbouring_field(%n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    store i64 %i, @pair
    %f = index i8 @pair, 8
    %next = binary add i64 %i, 1
    store i64 %next, %f
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# A location whose address the loop computes anew is not one location.
function @stepped(%n : i64, %p : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %at = index i64 %p, %i
    store typed i64 %i, %at
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# Not an x87 value or an i128.
function @large_values(%n : i64, %v : f64) -> void [no_inline=yes] {
  slot $w : i128
  block ^entry:
    %e = convert fpext f80 f64 %v
    %s = convert sext i128 i64 %n
    %p = addr $w
    %q = call i64 @opaque(%n)
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %a = index i8 @pair, 0
    store f80 %e, %a
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}
