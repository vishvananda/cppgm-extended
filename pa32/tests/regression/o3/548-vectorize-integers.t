# One-block counted loops of integer lanes. The test of the arrays' ranges stands
# before the wide loop, and the scalar loop finishes.

function @add_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %r = binary add i8 %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @sub_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %r = binary sub i8 %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @and_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %r = binary and i8 %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @or_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %r = binary or i8 %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @xor_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %r = binary xor i8 %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @add_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %r = binary add i16 %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @sub_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %r = binary sub i16 %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @and_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %r = binary and i16 %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @or_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %r = binary or i16 %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @xor_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %r = binary xor i16 %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @add_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = binary add i32 %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @sub_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = binary sub i32 %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @and_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = binary and i32 %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @or_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = binary or i32 %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @xor_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = binary xor i32 %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @add_i64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %q = index i64 %y, %i
    %w = load typed i64 %q
    %r = binary add i64 %v, %w
    %o = index i64 %d, %i
    store typed i64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @sub_i64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %q = index i64 %y, %i
    %w = load typed i64 %q
    %r = binary sub i64 %v, %w
    %o = index i64 %d, %i
    store typed i64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @and_i64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %q = index i64 %y, %i
    %w = load typed i64 %q
    %r = binary and i64 %v, %w
    %o = index i64 %d, %i
    store typed i64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @or_i64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %q = index i64 %y, %i
    %w = load typed i64 %q
    %r = binary or i64 %v, %w
    %o = index i64 %d, %i
    store typed i64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @xor_i64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %q = index i64 %y, %i
    %w = load typed i64 %q
    %r = binary xor i64 %v, %w
    %o = index i64 %d, %i
    store typed i64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @mul_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %r = binary mul i16 %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @mul_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = binary mul i32 %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @shl_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %r = binary shl i16 %v, 3
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @shr_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %r = binary shr i16 %v, 3
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @ushr_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %r = binary ushr i16 %v, 3
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @shl_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = binary shl i32 %v, 3
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @shr_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = binary shr i32 %v, 3
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @ushr_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = binary ushr i32 %v, 3
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @shl_i64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %q = index i64 %y, %i
    %w = load typed i64 %q
    %r = binary shl i64 %v, 3
    %o = index i64 %d, %i
    store typed i64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @ushr_i64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %q = index i64 %y, %i
    %w = load typed i64 %q
    %r = binary ushr i64 %v, 3
    %o = index i64 %d, %i
    store typed i64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @neg_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = unary neg i32 %v
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @bitnot_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %r = unary bitnot i32 %v
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_eq_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %t = cmp eq i8 %v, %w
    %r = select i8 %t, %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_ne_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %t = cmp ne i8 %v, %w
    %r = select i8 %t, %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_lt_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %t = cmp lt i8 %v, %w
    %r = select i8 %t, %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_le_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %t = cmp le i8 %v, %w
    %r = select i8 %t, %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_gt_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %t = cmp gt i8 %v, %w
    %r = select i8 %t, %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_ge_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %t = cmp ge i8 %v, %w
    %r = select i8 %t, %v, %w
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_eq_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %t = cmp eq i16 %v, %w
    %r = select i16 %t, %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_ne_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %t = cmp ne i16 %v, %w
    %r = select i16 %t, %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_lt_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %t = cmp lt i16 %v, %w
    %r = select i16 %t, %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_le_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %t = cmp le i16 %v, %w
    %r = select i16 %t, %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_gt_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %t = cmp gt i16 %v, %w
    %r = select i16 %t, %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_ge_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %t = cmp ge i16 %v, %w
    %r = select i16 %t, %v, %w
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_eq_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %t = cmp eq i32 %v, %w
    %r = select i32 %t, %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_ne_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %t = cmp ne i32 %v, %w
    %r = select i32 %t, %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_lt_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %t = cmp lt i32 %v, %w
    %r = select i32 %t, %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_le_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %t = cmp le i32 %v, %w
    %r = select i32 %t, %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_gt_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %t = cmp gt i32 %v, %w
    %r = select i32 %t, %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @select_ge_i32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %q = index i32 %y, %i
    %w = load typed i32 %q
    %t = cmp ge i32 %v, %w
    %r = select i32 %t, %v, %w
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @literal_i8(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %v = load i8 %p
    %q = index i8 %y, %i
    %w = load i8 %q
    %r = binary add i8 %v, 3
    %o = index i8 %d, %i
    store i8 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @literal_i16(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %q = index i16 %y, %i
    %w = load typed i16 %q
    %r = binary add i16 %v, 300
    %o = index i16 %d, %i
    store typed i16 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @literal_i64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %q = index i64 %y, %i
    %w = load typed i64 %q
    %r = binary add i64 %v, 3000000000
    %o = index i64 %d, %i
    store typed i64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
