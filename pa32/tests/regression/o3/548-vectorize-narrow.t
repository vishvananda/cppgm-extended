# Wide arithmetic only a truncation reads runs in narrow lanes, sixteen to a
# register for bytes.

function @add_bytes(%d : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index u8 %x, %i
    %a = load u8 %p
    %q = index u8 %y, %i
    %b = load u8 %q
    %wa = convert zext i32 u8 %a
    %wb = convert zext i32 u8 %b
    %s = binary add i32 %wa, %wb
    %t = convert trunc u8 i32 %s
    %o = index u8 %d, %i
    store u8 %t, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @sum_bytes(%x : ptr, %n : i64) -> i8 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i8 [^entry: -3, ^body: %t]
    %p = index i8 %x, %i
    %a = load i8 %p
    %w = convert sext i32 i8 %a
    %m = binary and i32 %w, 10
    %ws = convert sext i32 i8 %s
    %sum = binary add i32 %ws, %m
    %t = convert trunc i8 i32 %sum
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i8 %t
}

function @sum_shorts(%x : ptr, %n : i64) -> u16 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi u16 [^entry: 7, ^body: %t]
    %p = index u16 %x, %i
    %a = load typed u16 %p
    %w = convert zext i32 u16 %a
    %m = binary mul i32 %w, 3
    %ws = convert zext i32 u16 %s
    %sum = binary add i32 %ws, %m
    %t = convert trunc u16 i32 %sum
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return u16 %t
}

# Stays in i32 lanes: there is no byte multiply.
function @mul_bytes(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index u8 %x, %i
    %a = load u8 %p
    %w = convert zext i32 u8 %a
    %m = binary mul i32 %w, 3
    %t = convert trunc u8 i32 %m
    %o = index u8 %d, %i
    store u8 %t, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Stays in i32 lanes: the wide sum has another reader.
function @both(%d : ptr, %e : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index u8 %x, %i
    %a = load u8 %p
    %w = convert zext i32 u8 %a
    %s = binary add i32 %w, 300
    %o = index i32 %e, %i
    store i32 %s, %o
    %t = convert trunc u8 i32 %s
    %q = index u8 %d, %i
    store u8 %t, %q
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Signed bytes into an unsigned byte: the same bits under another type.
function @other_sign(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i8 %x, %i
    %a = load i8 %p
    %w = convert sext i32 i8 %a
    %s = binary add i32 %w, 1
    %t = convert trunc u8 i32 %s
    %o = index u8 %d, %i
    store u8 %t, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: a copy of the running value is stored each pass.
function @running(%d : ptr, %x : ptr, %n : i64) -> i32 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i32 [^entry: 0, ^body: %sum]
    %p = index i32 %x, %i
    %a = load i32 %p
    %sum = binary add i32 %s, %a
    %t = copy i32 %sum
    %u = binary add i32 %t, 1
    %o = index i32 %d, %i
    store i32 %u, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i32 %sum
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
