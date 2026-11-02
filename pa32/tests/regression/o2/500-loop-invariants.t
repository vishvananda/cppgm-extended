# Pure computations of operands a loop does not change move to its preheader.

declare function @may_throw(%x : i64) -> i64

function @opaque(%x : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary add i64 %x, 1
    return i64 %r
}

# A product and the sum that reads it: the second moves once the first has.
function @chain(%n : i64, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %s = phi i64 [^entry: 0, ^tail: %t]
    %c = cmp ult i64 %i, %n
    branch %c, ^body, ^done
  block ^body:
    %m = binary mul i64 %a, %b
    %q = binary add i64 %m, 7
    %t = binary add i64 %s, %q
    jump ^tail
  block ^tail:
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return i64 %s
}

# The product reads the counter.
function @varying(%n : i64, %a : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %s = phi i64 [^entry: 0, ^tail: %t]
    %c = cmp ult i64 %i, %n
    branch %c, ^tail, ^done
  block ^tail:
    %m = binary mul i64 %a, %i
    %t = binary add i64 %s, %m
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return i64 %s
}

# A loop inside a loop: the product leaves both.
function @nested(%n : i64, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^outer
  block ^outer:
    %i = phi i64 [^entry: 0, ^outer_tail: %i_next]
    %s = phi i64 [^entry: 0, ^outer_tail: %s_out]
    %c = cmp ult i64 %i, %n
    branch %c, ^inner_entry, ^done
  block ^inner_entry:
    jump ^inner
  block ^inner:
    %j = phi i64 [^inner_entry: 0, ^inner_tail: %j_next]
    %u = phi i64 [^inner_entry: %s, ^inner_tail: %v]
    %d = cmp ult i64 %j, %i
    branch %d, ^inner_tail, ^outer_tail
  block ^inner_tail:
    %m = binary mul i64 %a, %b
    %v = binary add i64 %u, %m
    %j_next = binary add i64 %j, 1
    jump ^inner
  block ^outer_tail:
    %s_out = phi i64 [^inner: %u]
    %i_next = binary add i64 %i, 1
    jump ^outer
  block ^done:
    return i64 %s
}

# The entry block branches past the loop: a preheader takes what moves.
function @entered_from_branch(%n : i64, %a : i64, %b : i64, %f : i64, %p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %z = cmp eq i64 %f, 0
    branch %z, ^head, ^other
  block ^other:
    %r = call i64 @opaque(%a)
    %v = binary add i64 %r, %b
    return i64 %v
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %s = phi i64 [^entry: 0, ^tail: %t]
    %c = cmp ult i64 %i, %n
    branch %c, ^body, ^done
  block ^body:
    %m = binary mul i64 %a, %b
    %y = cmp ugt i64 %i, %m
    branch %y, ^note, ^tail
  block ^note:
    store i64 %i, %p
    jump ^tail
  block ^tail:
    %t = binary add i64 %s, %m
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return i64 %s
}

# The sum in a later block moves after the product in an earlier one.
function @chain_across_blocks(%n : i64, %a : i64, %b : i64, %f : i64, %p : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^second: %next]
    %c = cmp ult i64 %i, %n
    branch %c, ^first, ^done
  block ^first:
    %m = binary mul i64 %a, %b
    %z = cmp eq i64 %f, %i
    branch %z, ^done, ^second
  block ^second:
    %q = binary add i64 %m, 7
    store i64 %q, %p
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    %e = binary add i64 %a, %b
    %w = binary add i64 %e, %m
    store i64 %w, %p
    return void
}

# The division first reaches the outer header, where it is certain too.
function @nested_certain_division(%n : i64, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^outer
  block ^outer:
    %i = phi i64 [^entry: 0, ^latch: %i_next]
    %s = phi i64 [^entry: 0, ^latch: %t]
    jump ^inner
  block ^inner:
    %j = phi i64 [^outer: 0, ^inner: %j_next]
    %u = phi i64 [^outer: %s, ^inner: %t]
    %q = binary div i64 %a, %b
    %t = binary add i64 %u, %q
    %j_next = binary add i64 %j, 1
    %d = cmp ult i64 %j_next, %i
    branch %d, ^inner, ^latch
  block ^latch:
    %i_next = binary add i64 %i, 1
    %c = cmp ult i64 %i_next, %n
    branch %c, ^outer, ^done
  block ^done:
    return i64 %t
}

# A division on a path that might not run stays, unless nothing can trap: the
# divisor is a literal other than 0 and, signed, other than -1.
function @guarded_division(%n : i64, %a : i64, %b : i64, %f : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %s = phi i64 [^entry: 0, ^tail: %t]
    %c = cmp ult i64 %i, %n
    branch %c, ^body, ^done
  block ^body:
    %z = cmp eq i64 %f, %i
    branch %z, ^divide, ^tail
  block ^divide:
    %q1 = binary div i64 %a, %b
    %q2 = binary udiv i64 %a, %b
    %q3 = binary mod i64 %a, %b
    %q4 = binary umod i64 %a, %b
    %q5 = binary div i64 %a, 3
    %q6 = binary div i64 %a, -1
    %q7 = binary udiv i64 %a, 0
    %q8 = binary mod i64 %a, -1
    %x1 = binary add i64 %q1, %q2
    %x2 = binary add i64 %x1, %q3
    %x3 = binary add i64 %x2, %q4
    %x4 = binary add i64 %x3, %q5
    %x5 = binary add i64 %x4, %q6
    %x6 = binary add i64 %x5, %q7
    %x7 = binary add i64 %x6, %q8
    jump ^tail
  block ^tail:
    %t = phi i64 [^body: %s, ^divide: %x7]
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return i64 %s
}

# Every entry to the loop reaches this division: it moves.
function @certain_division(%n : i64, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %s = phi i64 [^entry: 0, ^head: %t]
    %q = binary div i64 %a, %b
    %t = binary add i64 %s, %q
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return i64 %t
}

# A call that might not return comes first: the division stays.
function @division_after_call(%n : i64, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %s = phi i64 [^entry: 0, ^head: %t]
    %r = call i64 @opaque(%i)
    %q = binary div i64 %a, %b
    %w = binary add i64 %r, %q
    %t = binary add i64 %s, %w
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return i64 %t
}

# Across a call only the costly product moves.
function @costly_across_calls(%n : i64, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %s = phi i64 [^entry: 0, ^tail: %t]
    %c = cmp ult i64 %i, %n
    branch %c, ^tail, ^done
  block ^tail:
    %r = call i64 @opaque(%i)
    %m = binary mul i64 %a, %b
    %p = binary add i64 %a, %b
    %x = binary add i64 %m, %p
    %y = binary add i64 %r, %x
    %t = binary add i64 %s, %y
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return i64 %s
}

global @table : obj<64x8> = { zero 64 }

# The global's address and a literal step fit in the load's operand; a step by
# a variable takes a register, so it moves.
function @addresses(%n : i64, %p : ptr, %k : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %s = phi i64 [^entry: 0, ^tail: %t]
    %c = cmp ult i64 %i, %n
    branch %c, ^tail, ^done
  block ^tail:
    %g = index i8 @table, 8
    store i64 %i, %g
    %q = index i8 %p, %k
    %w = load i64 %q
    %t = binary add i64 %s, %w
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return i64 %s
}

# A slot's address that reaches only a store stays beside it; one a variable
# step reads takes a register, also through a literal step, so it moves.
function @slot_addresses(%n : i64) -> i64 [no_inline=yes] {
  slot $s : obj<64x8>
  slot $v : obj<64x8>
  slot $w : obj<64x8>
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %c = cmp ult i64 %i, %n
    branch %c, ^tail, ^done
  block ^tail:
    %p = addr $s
    store volatile i64 %i, %p
    %q = addr $v
    %e = binary and i64 %i, 7
    %x = index i64 %q, %e
    store volatile i64 %i, %x
    %r = addr $w
    %f = index i8 %r, 8
    %g = binary and i64 %i, 3
    %y = index i64 %f, %g
    store volatile i64 %i, %y
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return i64 %i
}

# An x87 value would live in memory: it stays.
function @extended(%n : i64, %a : f64, %b : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %x = convert fpext f80 f64 %a
    %y = convert fpext f80 f64 %b
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %s = phi f64 [^entry: 0.0, ^tail: %t]
    %c = cmp ult i64 %i, %n
    branch %c, ^tail, ^done
  block ^tail:
    %m = binary mul f80 %x, %y
    %u = convert fptrunc f64 f80 %m
    %t = binary add f64 %s, %u
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return f64 %s
}

# A loop holding a handler is out of reach.
function @handler_stays(%n : i64, %a : i64, %b : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %s = phi i64 [^entry: 0, ^tail: %u]
    %c = cmp ult i64 %i, %n
    branch %c, ^body, ^done
  block ^body:
    eh_try ^caught
    %r = call i64 @may_throw(%i)
    eh_end
    jump ^tail
  block ^caught:
    %m = binary mul i64 %a, %b
    jump ^tail
  block ^tail:
    %t = phi i64 [^body: %r, ^caught: %m]
    %u = binary add i64 %s, %t
    %next = binary add i64 %i, 1
    jump ^head
  block ^done:
    return i64 %s
}
