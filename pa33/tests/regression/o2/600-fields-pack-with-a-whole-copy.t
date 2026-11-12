# An object stored a field at a time, copied whole, and read from the copy
# as one 64-bit integer (a by-value argument after its call is inlined: the
# callee reads the first eightbyte whole). The copy holds that eightbyte as
# one integer, so the source's fields there pack into one too: both objects
# split, and the read is the stored values put together. Bytes no store
# writes stay out of the result.
global @seen : i64 = 0

# u32 at 0, four bytes of padding, a pointer at 8, u32 at 16.
function @argument(%a : u32, %p : ptr) -> i64 [binding=strong, no_inline=yes] {
  slot $arg : obj<24x8>
  slot $x : obj<24x8>
  block ^entry:
    %base = addr $arg
    %second = index i8 [projection=field] %base, 8
    %third = index i8 [projection=field] %base, 16
    store typed u32 %a, %base
    store typed ptr %p, %second
    store typed u32 %a, %third
    %copy = addr $x
    copyobj 24x8 $arg, %copy
    %word = load i64 %copy
    %low = binary and i64 %word, 4294967295
    %where = index i8 [projection=field] %copy, 8
    %q = load typed ptr %where
    %there = load i64 %q
    %r = binary add i64 %low, %there
    return i64 %r
}

# Both halves stored: the whole read is the two values, shifted and joined.
function @both_halves(%a : u32, %b : u32) -> i64 [binding=strong, no_inline=yes] {
  slot $arg : obj<24x8>
  slot $x : obj<24x8>
  block ^entry:
    %base = addr $arg
    %high = index i8 [projection=field] %base, 4
    %next = index i8 [projection=field] %base, 8
    %third = index i8 [projection=field] %base, 16
    store typed u32 %a, %base
    store typed u32 %b, %high
    store typed i64 7, %next
    store typed u32 %b, %third
    %copy = addr $x
    copyobj 24x8 $arg, %copy
    %word = load i64 %copy
    %tail = index i8 [projection=field] %copy, 8
    %seven = load typed i64 %tail
    %r = binary add i64 %word, %seven
    return i64 %r
}

# Two copies in a row: the middle object packs with the last, and the first
# with the middle.
function @chain(%a : u32, %b : u32) -> i64 [binding=strong, no_inline=yes] {
  slot $arg : obj<24x8>
  slot $x : obj<24x8>
  slot $y : obj<24x8>
  block ^entry:
    %base = addr $arg
    %high = index i8 [projection=field] %base, 4
    %next = index i8 [projection=field] %base, 8
    %third = index i8 [projection=field] %base, 16
    store typed u32 %a, %base
    store typed u32 %b, %high
    store typed i64 9, %next
    store typed u32 %a, %third
    %middle = addr $x
    copyobj 24x8 $arg, %middle
    %last = addr $y
    copyobj 24x8 $x, %last
    %word = load i64 %last
    %tail = index i8 [projection=field] %last, 16
    %again = load typed u32 %tail
    %wide = convert zext i64 u32 %again
    %r = binary add i64 %word, %wide
    return i64 %r
}

# A floating field does not pack with an integer: both objects stay in
# memory, and the copy stays.
function @floating(%a : f32, %b : u32) -> i64 [binding=strong, no_inline=yes] {
  slot $arg : obj<24x8>
  slot $x : obj<24x8>
  block ^entry:
    %base = addr $arg
    %high = index i8 [projection=field] %base, 4
    %next = index i8 [projection=field] %base, 8
    %third = index i8 [projection=field] %base, 16
    store typed f32 %a, %base
    store typed u32 %b, %high
    store typed i64 9, %next
    store typed u32 %b, %third
    %copy = addr $x
    copyobj 24x8 $arg, %copy
    %word = load i64 %copy
    return i64 %word
}

function @main() -> i32 [role=entry] {
  block ^entry:
    store i64 40, @seen
    %a = call i64 @argument(5, @seen)
    %b = call i64 @both_halves(3, 1)
    %c = call i64 @chain(2, 1)
    %d = call i64 @floating(1.0, 1)
    %b1 = binary ushr i64 %b, 32
    %c1 = binary ushr i64 %c, 30
    %d1 = binary ushr i64 %d, 28
    %s1 = binary add i64 %a, %b
    %s2 = binary add i64 %s1, %b1
    %s3 = binary add i64 %s2, %c
    %s4 = binary add i64 %s3, %c1
    %s5 = binary add i64 %s4, %d1
    %r = convert trunc i32 i64 %s5
    %u = binary and i32 %r, 255
    return i32 %u
}
