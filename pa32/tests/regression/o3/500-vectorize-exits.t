# The counted forms the wide loop runs: the bound where its lanes must stop.

function @up_lt(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @up_le(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp le i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @up_ult(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @up_ule(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp ule i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @up_ne(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp ne i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @flipped(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp gt i64 %n, %next
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @stays_on_false(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp ge i64 %next, %n
    branch %c, ^done, ^body
  block ^done:
    return void
}

function @tests_old_value(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %i, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @narrow_counter(%d : ptr, %x : ptr, %n : i32) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add nowrap i32 %i, 1
    %c = cmp lt i32 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @narrow_unsigned(%d : ptr, %x : ptr, %n : u32) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi u32 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add u32 %i, 1
    %c = cmp ult u32 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @counts_down(%d : ptr, %x : ptr, %n : i32) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: %n, ^body: %next]
    %a = phi ptr [^entry: %x, ^body: %a2]
    %b = phi ptr [^entry: %d, ^body: %b2]
    %v = load typed i32 %a
    %r = binary add i32 %v, 1
    store typed i32 %r, %b
    %a2 = index i32 %a, 1
    %b2 = index i32 %b, 1
    %next = binary sub nowrap i32 %i, 1
    branch %next, ^body, ^done
  block ^done:
    return void
}

function @pointer_counter(%d : ptr, %e : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %p = phi ptr [^entry: %d, ^body: %q]
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    store typed i32 %r, %p
    %q = index i32 %p, 1
    %c = cmp ne ptr %q, %e
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: a step of two and the bound a load changes.
function @step_two(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 2
    %c = cmp ne i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
