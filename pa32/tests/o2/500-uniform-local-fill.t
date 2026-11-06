# Integer content facts must respect every other observation and write.
declare function @memset(%p : ptr, %byte : i32, %size : i64) -> ptr [unwind=no, linkage=c, binding=strong]

function @uniform_word(%index : i64) -> i64 [no_inline=yes] {
  slot $value : obj<64x8>
  block ^entry:
    %p = addr $value
    %filled = call ptr @memset(%p, 165, 64)
    %bounded = binary and i64 %index, 7
    %q = index i64 %p, %bounded
    %result = load i64 %q
    return i64 %result
}

function @uniform_short(%index : i64) -> i16 [no_inline=yes] {
  slot $value : obj<64x8>
  block ^entry:
    %p = addr $value
    %filled = call ptr @memset(%p, -1, 64)
    %bounded = binary and i64 %index, 31
    %q = index i16 %p, %bounded
    %result = load i16 %q
    return i16 %result
}

function @uniform_byte(%index : i64) -> u8 [no_inline=yes] {
  slot $value : obj<16x8>
  block ^entry:
    %p = addr $value
    %filled = call ptr @memset(%p, 319, 16)
    %bounded = binary and i64 %index, 15
    %q = index i8 %p, %bounded
    %result = load u8 %q
    return u8 %result
}

function @partial_write(%index : i64, %byte : i8) -> i64 [no_inline=yes] {
  slot $value : obj<16x8>
  block ^entry:
    %p = addr $value
    %filled = call ptr @memset(%p, 165, 16)
    %bounded = binary and i64 %index, 7
    %q = index i8 %p, %bounded
    store i8 %byte, %q
    %result = load i64 %p
    return i64 %result
}

function @merged_address(%choose : i64) -> i32 [no_inline=yes] {
  slot $first : obj<16x8>
  slot $second : obj<16x8>
  block ^entry:
    %a = addr $first
    %b = addr $second
    %filled_a = call ptr @memset(%a, 18, 16)
    %filled_b = call ptr @memset(%b, 52, 16)
    branch %choose, ^take_a, ^take_b
  block ^take_a:
    jump ^join
  block ^take_b:
    jump ^join
  block ^join:
    %p = phi ptr [^take_a: %a, ^take_b: %b]
    %result = load i32 %p
    return i32 %result
}

function @read_before_fill(%seed : i64) -> i64 [no_inline=yes] {
  slot $value : obj<16x8>
  block ^entry:
    %p = addr $value
    store i64 %seed, %p
    %before = load i64 %p
    %filled = call ptr @memset(%p, 165, 16)
    %after = load i64 %p
    %result = binary xor i64 %before, %after
    return i64 %result
}

function @volatile_fill() -> i32 [no_inline=yes] {
  slot $value : obj<16x8>
  block ^entry:
    %p = addr $value
    %filled = call ptr @memset(%p, 19, 16)
    %result = load volatile i32 %p
    return i32 %result
}

function @atomic_fill() -> i32 [no_inline=yes] {
  slot $value : obj<16x8>
  block ^entry:
    %p = addr $value
    %filled = call ptr @memset(%p, 23, 16)
    %result = atomic_load i32 %p, 5
    return i32 %result
}

function @floating_fill() -> f32 [no_inline=yes] {
  slot $value : obj<16x8>
  block ^entry:
    %p = addr $value
    %filled = call ptr @memset(%p, 63, 16)
    %result = load f32 %p
    return f32 %result
}

function @wide_fill() -> i64 [no_inline=yes] {
  slot $value : obj<16x16>
  block ^entry:
    %p = addr $value
    %filled = call ptr @memset(%p, 1, 16)
    %whole = load i128 %p
    %high = binary ushr i128 %whole, 64
    %low_word = convert trunc i64 i128 %whole
    %high_word = convert trunc i64 i128 %high
    %result = binary xor i64 %low_word, %high_word
    return i64 %result
}

function @copy_source() -> i64 [no_inline=yes] {
  slot $value : obj<16x8>
  slot $copy : obj<16x8>
  block ^entry:
    %p = addr $value
    %q = addr $copy
    %filled = call ptr @memset(%p, 51, 16)
    copyobj 16x8 %p, %q
    %result = load i64 %q
    return i64 %result
}

function @conditional_refill(%choose : i64) -> i64 [no_inline=yes] {
  slot $value : obj<16x8>
  block ^entry:
    %p = addr $value
    %filled = call ptr @memset(%p, 165, 16)
    branch %choose, ^refill, ^join
  block ^refill:
    %other = call ptr @memset(%p, 13, 16)
    jump ^join
  block ^join:
    %result = load i64 %p
    return i64 %result
}

function @repeated_fill() -> i64 [no_inline=yes] {
  slot $value : obj<16x8>
  block ^entry:
    %p = addr $value
    %first = call ptr @memset(%p, 165, 16)
    %second = call ptr @memset(%p, 79, 16)
    %result = load i64 %p
    return i64 %result
}
