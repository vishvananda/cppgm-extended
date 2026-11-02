# Loops that walk their arrays down. A lane holds its pass with the first pass
# on top, so the accesses keep to the same order and nothing is shuffled.

function @add_down(%y : ptr, %x : ptr, %n : i32) -> void [no_inline=yes] {
  block ^entry:
    %first = binary sub nowrap i32 %n, 1
    jump ^body
  block ^body:
    %i = phi i32 [^entry: %first, ^body: %next]
    %w = convert sext i64 i32 %i
    %p = index i32 %x, %w
    %a = load typed i32 %p
    %q = index f32 %y, %w
    %b = load typed f32 %q
    %c = convert sitofp f32 i32 %a
    %s = binary add f32 %b, %c
    store typed f32 %s, %q
    %next = binary sub nowrap i32 %i, 1
    %t = cmp ge i32 %next, 0
    branch %t, ^body, ^done
  block ^done:
    return void
}

function @pointer_down(%first : ptr, %last : ptr) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %p = phi ptr [^entry: %last, ^body: %next]
    %next = index i8 %p, -8
    %v = load typed i64 %next
    %w = binary xor i64 %v, 5
    store typed i64 %w, %next
    %c = cmp ne ptr %next, %first
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Bytes may alias the words, so the ranges are tested.
function @bytes_down(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: %n, ^body: %next]
    %p = index u8 %x, %i
    %v = load u8 %p
    %w = convert zext i32 u8 %v
    %o = index i32 %d, %i
    store i32 %w, %o
    %next = binary sub i64 %i, 1
    %c = cmp gt i64 %next, 0
    branch %c, ^body, ^done
  block ^done:
    return void
}

# The counter is data: its lanes go down the register.
function @counter_down(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: 100, ^body: %next]
    %k = phi i64 [^entry: %n, ^body: %knext]
    %o = index i32 %d, %k
    store typed i32 %i, %o
    %next = binary add nowrap i32 %i, 3
    %knext = binary sub i64 %k, 1
    %c = cmp gt i64 %knext, 0
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @long_counter_down(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: %n, ^body: %next]
    %o = index i64 %d, %i
    store typed i64 %i, %o
    %next = binary sub i64 %i, 1
    %c = cmp gt i64 %next, 0
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @sum_down(%x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: %n, ^body: %next]
    %s = phi i32 [^entry: 0, ^body: %sum]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %sum = binary add i32 %s, %v
    %next = binary sub i64 %i, 1
    %c = cmp gt i64 %next, 0
    branch %c, ^body, ^done
  block ^done:
    return i32 %sum
}

# Shifts an array up in place: each pass reads what is ahead of the next write.
function @shift_up(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: %n, ^body: %next]
    %p = index i32 %d, %i
    %q = index i8 %p, -4
    %v = load typed i32 %q
    store typed i32 %v, %p
    %next = binary sub i64 %i, 1
    %c = cmp gt i64 %next, 0
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: the store is ahead of the load of one pass later.
function @shift_down(%d : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: %n, ^body: %next]
    %p = index i32 %d, %i
    %q = index i8 %p, -4
    %v = load typed i32 %p
    store typed i32 %v, %q
    %next = binary sub i64 %i, 1
    %c = cmp gt i64 %next, 0
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: one access up, one down.
function @reverse_copy(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %m = binary sub i64 %n, %i
    %o = index i32 %d, %m
    store typed i32 %v, %o
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
