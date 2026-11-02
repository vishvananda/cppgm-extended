# Values of two sizes in one loop. The widest sets the lanes; a narrower value
# keeps as many lanes at the bottom of the register.

function @widen_short(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %x, %i
    %v = load typed i16 %p
    %w = convert sext i32 i16 %v
    %o = index i32 %d, %i
    store typed i32 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @widen_byte(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index u8 %x, %i
    %v = load u8 %p
    %w = convert zext i32 u8 %v
    %o = index i32 %d, %i
    store typed i32 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @narrow(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %w = convert trunc i16 i32 %v
    %o = index i16 %d, %i
    store typed i16 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @to_double(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %w = convert sitofp f64 i32 %v
    %o = index f64 %d, %i
    store typed f64 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @to_float(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %w = convert fptrunc f32 f64 %v
    %o = index f32 %d, %i
    store typed f32 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Each wider element is a sum of a sign-extended narrower one.
function @two_arrays(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %w = convert sext i64 i32 %v
    %t = binary add i64 %w, 7
    %o = index i64 %d, %i
    store typed i64 %t, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Nothing converts between the two sizes.
function @two_sizes(%a : ptr, %b : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i16 %a, %i
    store typed i16 1, %p
    %q = index i32 %b, %i
    store typed i32 2, %q
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @to_int(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %w = convert fptosi i32 f64 %v
    %o = index i32 %d, %i
    store typed i32 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Two lanes of two bytes each.
function @long_to_short(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %w = convert trunc i16 i64 %v
    %o = index i16 %d, %i
    store typed i16 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Two lanes of one byte each.
function @long_to_byte(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %w = convert trunc u8 i64 %v
    %o = index u8 %d, %i
    store u8 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Elements of two sizes from one object need a range test as long as the wider.
function @two_sizes_tested(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %a = index u8 %x, %i
    %v = load u8 %a
    %w = convert zext i32 u8 %v
    %b = index i32 %x, %i
    %u = load i32 %b
    %s = binary add i32 %w, %u
    %o = index i32 %d, %i
    store i32 %s, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: the same offset of one array steps by two sizes.
function @same_offset(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %a = index u8 %d, %i
    %v = load u8 %a
    %w = convert zext i32 u8 %v
    %o = index i32 %d, %i
    store i32 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: a load ahead of a store of another size is passed by it.
function @ahead(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %b = index i8 %d, 8
    %a = index u8 %b, %i
    %v = load u8 %a
    %w = convert zext i32 u8 %v
    %o = index i32 %d, %i
    store i32 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: closer than a wide pass of the wider size.
function @close(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %b = index i8 %d, 8
    %o = index i32 %b, %i
    %a = index u8 %d, %i
    %v = load u8 %a
    %w = convert zext i32 u8 %v
    store i32 %w, %o
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
