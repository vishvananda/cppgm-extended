# A call to a function that writes nothing its callers can see hides nothing:
# the load after it reads the load before, and the store before it stays,
# since it reads. Writes to its own slot and calls in a cycle do not count
# against a function. Writing through a parameter, calling a function that
# does and being replaceable do.
global @cell : i64 = 3

function @peek(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %v = load typed i64 %p
    return i64 %v
}

function @own_slot(%p : ptr, %n : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %a = addr $s
    %v = load typed i64 %p
    store typed i64 %v, %a
    %b = index i8 %a, 8
    store typed i64 1, %b
    %e = index i64 %a, %n
    store typed i64 2, %e
    %w = load typed i64 %a
    return i64 %w
}

function @odd(%p : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %z = cmp eq i64 %n, 0
    branch %z, ^done, ^more
  block ^done:
    return i64 0
  block ^more:
    %m = binary sub i64 %n, 1
    %r = call i64 @even(%p, %m)
    return i64 %r
}

function @even(%p : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %z = cmp eq i64 %n, 0
    branch %z, ^done, ^more
  block ^done:
    %v = load typed i64 %p
    return i64 %v
  block ^more:
    %m = binary sub i64 %n, 1
    %r = call i64 @odd(%p, %m)
    return i64 %r
}

function @poke(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    store typed i64 9, %p
    return i64 9
}

function @calls_poke(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %r = call i64 @poke(%p)
    return i64 %r
}

function @hook(%p : ptr) -> i64 [binding=weak, replaceable=yes] {
  block ^entry:
    %v = load typed i64 %p
    return i64 %v
}

function @across_peek(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %a = load typed i64 %p
    %c = call i64 @peek(%p)
    %b = load typed i64 %p
    %s = binary add i64 %a, %b
    %r = binary add i64 %s, %c
    return i64 %r
}

function @across_own_slot(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %a = load typed i64 %p
    %c = call i64 @own_slot(%p, 1)
    %b = load typed i64 %p
    %s = binary add i64 %a, %b
    %r = binary add i64 %s, %c
    return i64 %r
}

function @across_cycle(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %a = load typed i64 %p
    %c = call i64 @even(%p, 4)
    %b = load typed i64 %p
    %s = binary add i64 %a, %b
    %r = binary add i64 %s, %c
    return i64 %r
}

function @across_poke(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %a = load typed i64 %p
    %c = call i64 @calls_poke(%p)
    %b = load typed i64 %p
    %s = binary add i64 %a, %b
    %r = binary add i64 %s, %c
    return i64 %r
}

function @across_hook(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %a = load typed i64 %p
    %c = call i64 @hook(%p)
    %b = load typed i64 %p
    %s = binary add i64 %a, %b
    %r = binary add i64 %s, %c
    return i64 %r
}

function @store_stays(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    store typed i64 7, %p
    %c = call i64 @peek(%p)
    store typed i64 8, %p
    return i64 %c
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @across_peek(@cell)
    %b = call i64 @across_own_slot(@cell)
    %c = call i64 @across_cycle(@cell)
    %d = call i64 @across_hook(@cell)
    %e = call i64 @store_stays(@cell)
    %f = call i64 @across_poke(@cell)
    %s1 = binary add i64 %a, %b
    %s2 = binary add i64 %c, %d
    %s3 = binary add i64 %e, %f
    %s4 = binary add i64 %s1, %s2
    %s5 = binary add i64 %s3, %s4
    %l = load typed i64 @cell
    %s6 = binary add i64 %s5, %l
    %t = convert trunc i32 i64 %s6
    return i32 %t
}
