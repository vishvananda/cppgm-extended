# Each pass of the first loop names one element of a local array: the copies
# fold it into a register. The same long pass over memory it does not own stays
# a loop.

function @local(%m : i64) -> i64 [no_inline=yes] {
  slot $a : obj<64x8>
  block ^entry:
    %base = addr $a
    store typed i64 %m, %base
    jump ^head
  block ^head:
    %k = phi i64 [^entry: 0, ^tail: %next]
    %c = cmp ult i64 %k, 8
    branch %c, ^body, ^done
  block ^body:
    %p = index i64 %base, %k
    %v = load typed i64 %p
    %odd = binary and i64 %k, 1
    %t = cmp eq i64 %odd, 0
    branch %t, ^even, ^tail
  block ^even:
    %w = binary mul i64 %v, 3
    %x0 = binary add i64 %w, %k
    %x1 = binary add i64 %x0, %k
    %x2 = binary add i64 %x1, %k
    %x3 = binary add i64 %x2, %k
    %x4 = binary add i64 %x3, %k
    %x5 = binary add i64 %x4, %k
    %x6 = binary add i64 %x5, %k
    %x7 = binary add i64 %x6, %k
    %x8 = binary add i64 %x7, %k
    %x9 = binary add i64 %x8, %k
    %x10 = binary add i64 %x9, %k
    %x11 = binary add i64 %x10, %k
    %x12 = binary add i64 %x11, %k
    %x13 = binary add i64 %x12, %k
    %x14 = binary add i64 %x13, %k
    %x15 = binary add i64 %x14, %k
    %x16 = binary add i64 %x15, %k
    %x17 = binary add i64 %x16, %k
    %x18 = binary add i64 %x17, %k
    %x19 = binary add i64 %x18, %k
    %x20 = binary add i64 %x19, %k
    %x21 = binary add i64 %x20, %k
    %x22 = binary add i64 %x21, %k
    %x23 = binary add i64 %x22, %k
    %x24 = binary add i64 %x23, %k
    %x25 = binary add i64 %x24, %k
    %x26 = binary add i64 %x25, %k
    %x27 = binary add i64 %x26, %k
    %x28 = binary add i64 %x27, %k
    %x29 = binary add i64 %x28, %k
    %x30 = binary add i64 %x29, %k
    %x31 = binary add i64 %x30, %k
    %x32 = binary add i64 %x31, %k
    %x33 = binary add i64 %x32, %k
    %x34 = binary add i64 %x33, %k
    %x35 = binary add i64 %x34, %k
    %x36 = binary add i64 %x35, %k
    %x37 = binary add i64 %x36, %k
    %x38 = binary add i64 %x37, %k
    %x39 = binary add i64 %x38, %k
    %x40 = binary add i64 %x39, %k
    %x41 = binary add i64 %x40, %k
    %x42 = binary add i64 %x41, %k
    %x43 = binary add i64 %x42, %k
    store typed i64 %x43, %p
    jump ^tail
  block ^tail:
    %next = binary add i64 %k, 1
    jump ^head
  block ^done:
    %r = load typed i64 %base
    return i64 %r
}

function @elsewhere(%base : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %k = phi i64 [^entry: 0, ^tail: %next]
    %c = cmp ult i64 %k, 8
    branch %c, ^body, ^done
  block ^body:
    %p = index i64 %base, %k
    %v = load typed i64 %p
    %odd = binary and i64 %k, 1
    %t = cmp eq i64 %odd, 0
    branch %t, ^even, ^tail
  block ^even:
    %w = binary mul i64 %v, 3
    %x0 = binary add i64 %w, %k
    %x1 = binary add i64 %x0, %k
    %x2 = binary add i64 %x1, %k
    %x3 = binary add i64 %x2, %k
    %x4 = binary add i64 %x3, %k
    %x5 = binary add i64 %x4, %k
    %x6 = binary add i64 %x5, %k
    %x7 = binary add i64 %x6, %k
    %x8 = binary add i64 %x7, %k
    %x9 = binary add i64 %x8, %k
    %x10 = binary add i64 %x9, %k
    %x11 = binary add i64 %x10, %k
    %x12 = binary add i64 %x11, %k
    %x13 = binary add i64 %x12, %k
    %x14 = binary add i64 %x13, %k
    %x15 = binary add i64 %x14, %k
    %x16 = binary add i64 %x15, %k
    %x17 = binary add i64 %x16, %k
    %x18 = binary add i64 %x17, %k
    %x19 = binary add i64 %x18, %k
    %x20 = binary add i64 %x19, %k
    %x21 = binary add i64 %x20, %k
    %x22 = binary add i64 %x21, %k
    %x23 = binary add i64 %x22, %k
    %x24 = binary add i64 %x23, %k
    %x25 = binary add i64 %x24, %k
    %x26 = binary add i64 %x25, %k
    %x27 = binary add i64 %x26, %k
    %x28 = binary add i64 %x27, %k
    %x29 = binary add i64 %x28, %k
    %x30 = binary add i64 %x29, %k
    %x31 = binary add i64 %x30, %k
    %x32 = binary add i64 %x31, %k
    %x33 = binary add i64 %x32, %k
    %x34 = binary add i64 %x33, %k
    %x35 = binary add i64 %x34, %k
    %x36 = binary add i64 %x35, %k
    %x37 = binary add i64 %x36, %k
    %x38 = binary add i64 %x37, %k
    %x39 = binary add i64 %x38, %k
    %x40 = binary add i64 %x39, %k
    %x41 = binary add i64 %x40, %k
    %x42 = binary add i64 %x41, %k
    %x43 = binary add i64 %x42, %k
    store typed i64 %x43, %p
    jump ^tail
  block ^tail:
    %next = binary add i64 %k, 1
    jump ^head
  block ^done:
    %r = load typed i64 %base
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
