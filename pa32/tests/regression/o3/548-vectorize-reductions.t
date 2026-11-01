# A reduction's lanes are added across after the wide loop, and the scalar loop
# starts from the result.

function @add_i32(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 5, ^body: %r]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %r
}

function @sub_i32(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 5, ^body: %r]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary sub i32 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %r
}

function @and_i32(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 5, ^body: %r]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary and i32 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %r
}

function @or_i32(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 5, ^body: %r]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary or i32 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %r
}

function @xor_i32(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 5, ^body: %r]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary xor i32 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %r
}

function @add_i64(%x : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: 5, ^body: %r]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %r = binary add i64 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %r
}

function @sub_i64(%x : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: 5, ^body: %r]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %r = binary sub i64 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %r
}

function @and_i64(%x : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: 5, ^body: %r]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %r = binary and i64 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %r
}

function @or_i64(%x : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: 5, ^body: %r]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %r = binary or i64 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %r
}

function @xor_i64(%x : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: 5, ^body: %r]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %r = binary xor i64 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %r
}

function @add_zero(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 0, ^body: %r]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %r
}

function @add_commuted(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 0, ^body: %r]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, %s
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %r
}

function @and_commuted(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 7, ^body: %r]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary and i32 %v, %s
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %r
}

function @from_parameter(%x : ptr, %n : i64, %k : i32) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: %k, ^body: %r]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %r
}

function @unsigned_sum(%x : ptr, %n : i64) -> u32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi u32 [^entry: 3, ^body: %r]
    %p = index u32 %x, %i
    %v = load typed u32 %p
    %r = binary add u32 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return u32 %r
}

# Refused: a floating sum depends on its order.
function @float_sum(%x : ptr, %n : i64) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi f64 [^entry: 0.0, ^body: %r]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %r = binary add f64 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return f64 %r
}

# Refused: the running value is read beside its operator.
function @prefix_sums(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 0, ^body: %t]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %t = binary add i32 %s, %v
    %o = index i32 %d, %i
    store typed i32 %t, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: the subtraction takes the running value second.
function @sub_from(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 0, ^body: %r]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary sub i32 %v, %s
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %r
}

# Refused: no sum of bytes across the lanes.
function @byte_sum(%x : ptr, %n : i64) -> i8 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i8 [^entry: 0, ^body: %r]
    %p = index i8 %x, %i
    %v = load i8 %p
    %r = binary add i8 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i8 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
