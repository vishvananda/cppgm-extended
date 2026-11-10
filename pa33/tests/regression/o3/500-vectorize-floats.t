# One-block counted loops of floating lanes, with what converts in place.

function @add_f32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %q = index f32 %y, %i
    %w = load typed f32 %q
    %r = binary add f32 %v, %w
    %o = index f32 %d, %i
    store typed f32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @sub_f32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %q = index f32 %y, %i
    %w = load typed f32 %q
    %r = binary sub f32 %v, %w
    %o = index f32 %d, %i
    store typed f32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @mul_f32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %q = index f32 %y, %i
    %w = load typed f32 %q
    %r = binary mul f32 %v, %w
    %o = index f32 %d, %i
    store typed f32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @div_f32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %q = index f32 %y, %i
    %w = load typed f32 %q
    %r = binary div f32 %v, %w
    %o = index f32 %d, %i
    store typed f32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @sqrt_f32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %q = index f32 %y, %i
    %w = load typed f32 %q
    %r = unary sqrt f32 %v
    %o = index f32 %d, %i
    store typed f32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @neg_f32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %q = index f32 %y, %i
    %w = load typed f32 %q
    %r = unary neg f32 %v
    %o = index f32 %d, %i
    store typed f32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @add_f64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %q = index f64 %y, %i
    %w = load typed f64 %q
    %r = binary add f64 %v, %w
    %o = index f64 %d, %i
    store typed f64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @sub_f64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %q = index f64 %y, %i
    %w = load typed f64 %q
    %r = binary sub f64 %v, %w
    %o = index f64 %d, %i
    store typed f64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @mul_f64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %q = index f64 %y, %i
    %w = load typed f64 %q
    %r = binary mul f64 %v, %w
    %o = index f64 %d, %i
    store typed f64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @div_f64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %q = index f64 %y, %i
    %w = load typed f64 %q
    %r = binary div f64 %v, %w
    %o = index f64 %d, %i
    store typed f64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @sqrt_f64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %q = index f64 %y, %i
    %w = load typed f64 %q
    %r = unary sqrt f64 %v
    %o = index f64 %d, %i
    store typed f64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @neg_f64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %q = index f64 %y, %i
    %w = load typed f64 %q
    %r = unary neg f64 %v
    %o = index f64 %d, %i
    store typed f64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @literal_f32(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %q = index f32 %y, %i
    %w = load typed f32 %q
    %r = binary mul f32 %v, 2.5f
    %o = index f32 %d, %i
    store typed f32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @literal_f64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %q = index f64 %y, %i
    %w = load typed f64 %q
    %r = binary mul f64 %v, 2.5
    %o = index f64 %d, %i
    store typed f64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @magnitude_f64(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f64 %x, %i
    %v = load typed f64 %p
    %q = index f64 %y, %i
    %w = load typed f64 %q
    %b = copy i64 %v
    %m = binary and i64 %b, 9223372036854775807
    %r = copy f64 %m
    %o = index f64 %d, %i
    store typed f64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @from_int(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %q = index f32 %y, %i
    %w = load typed f32 %q
    %a = load typed i32 %p
    %r = convert sitofp f32 i32 %a
    %o = index f32 %d, %i
    store typed f32 %r, %o
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
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %r = convert fptosi i32 f32 %v
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @float_chooses_int(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %q = index f32 %y, %i
    %w = load typed f32 %q
    %t = cmp lt f32 %v, %w
    %o = index i32 %d, %i
    %a = load typed i32 %o
    %b = binary add i32 %a, 1
    %r = select i32 %t, %a, %b
    store typed i32 %r, %o
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
