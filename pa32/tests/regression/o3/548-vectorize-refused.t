# What stays scalar, and a counter that is data.

declare function @touch(%x : i32) -> void [unwind=no]
global @shared : i32 = 0

function @counter_as_data(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: 3, ^body: %next]
    %j = phi i64 [^entry: 0, ^body: %j2]
    %m = binary mul i32 %i, 7
    %o = index i32 %d, %j
    store typed i32 %m, %o
    %next = binary add nowrap i32 %i, 2
    %j2 = binary add i64 %j, 1
    %c = cmp lt i64 %j2, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @long_counter_as_data(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %o = index i64 %d, %i
    store typed i64 %i, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: a call.
function @calls(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %o = index i32 %d, %i
    %v = load typed i32 %o
    call void @touch(%v)
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: a volatile access.
function @volatile_load(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %o = index i32 %d, %i
    %v = load volatile i32 %o
    %r = binary add i32 %v, 1
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: a stride of two elements.
function @strided(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = binary mul i64 %i, 2
    %o = index i32 %d, %j
    store typed i32 1, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: an element reached through a loaded index.
function @gather(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %q = index i32 %d, %v
    store typed i32 1, %q
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: no conversion of 64-bit integer lanes in the baseline.
function @from_long(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %w = convert sitofp f64 i64 %v
    %o = index f64 %d, %i
    store typed f64 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: no unsigned conversion in the baseline.
function @from_unsigned(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index u32 %x, %i
    %v = load typed u32 %p
    %w = convert uitofp f32 u32 %v
    %o = index f32 %d, %i
    store typed f32 %w, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: integer division.
function @divides(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %o = index i32 %d, %i
    %v = load typed i32 %o
    %r = binary div i32 %v, 3
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: a store of one place.
function @one_place(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load i32 %p
    store i32 %v, %d
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: two blocks.
function @two_blocks(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %o = index i32 %d, %i
    %v = load typed i32 %o
    branch %v, ^then, ^tail
  block ^then:
    store typed i32 0, %o
    jump ^tail
  block ^tail:
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: no compare of 64-bit integer lanes in the baseline.
function @select_i64(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i64 %x, %i
    %v = load typed i64 %p
    %t = cmp lt i64 %v, 5
    %r = select i64 %t, %v, 5
    %o = index i64 %d, %i
    store typed i64 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: the bound changes in the loop.
function @moving_bound(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %b = phi i64 [^entry: %n, ^body: %b2]
    %o = index i32 %d, %i
    store typed i32 1, %o
    %b2 = binary sub i64 %b, 1
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %b2
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: an index that may wrap and is not the counter.
function @wrapping_index(%d : ptr, %n : i64, %s : i32) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = phi i32 [^entry: %s, ^body: %j2]
    %w = convert sext i64 i32 %j
    %o = index i32 %d, %w
    store typed i32 1, %o
    %j2 = binary add i32 %j, 1
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: a sum of the counter that may wrap before it widens.
function @wrapping_step(%d : ptr, %n : i32) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: 0, ^body: %next]
    %a = binary add i32 %i, 2
    %w = convert sext i64 i32 %a
    %o = index i32 %d, %w
    store typed i32 1, %o
    %next = binary add nowrap i32 %i, 1
    %c = cmp lt i32 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: a signed counter read as unsigned jumps at zero.
function @zero_extended(%d : ptr, %s : i32, %n : i32) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: %s, ^body: %next]
    %w = convert zext i64 i32 %i
    %o = index i32 %d, %w
    store typed i32 1, %o
    %next = binary add nowrap i32 %i, 1
    %c = cmp lt i32 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# A second counter of its own step gives the index.
function @second_counter(%d : ptr, %n : i64, %s : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = phi i64 [^entry: %s, ^body: %j2]
    %o = index i32 %d, %j
    store typed i32 1, %o
    %j2 = binary add i64 %j, 1
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# An offset added to a counter that keeps to its range.
function @step_after_widening(%d : ptr, %n : i32) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: 0, ^body: %next]
    %w = convert sext i64 i32 %i
    %a = binary add i64 %w, 2
    %o = index i32 %d, %a
    store typed i32 1, %o
    %next = binary add nowrap i32 %i, 1
    %c = cmp lt i32 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: not equal to the bound on a step of two may pass it by.
function @counter_by_two(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = phi i64 [^entry: 0, ^body: %j2]
    %p = index i32 %d, %j
    store typed i32 1, %p
    %j2 = binary add i64 %j, 1
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
