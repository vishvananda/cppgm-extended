# Ordinary byte-aligned fields can read only the demanded representation.
# Snapshot, volatile, shared-use and non-byte-aligned controls retain meaning.
function @float_high(%p : ptr) -> f32 [no_inline=yes] {
  block ^entry:
    %word = load i64 %p
    %high = binary ushr i64 %word, 32
    %bits = convert trunc u32 i64 %high
    %value = copy f32 %bits
    return f32 %value
}
function @float_low(%p : ptr) -> f32 [no_inline=yes] {
  block ^entry:
    %word = load i64 %p
    %bits = convert trunc u32 i64 %word
    %value = copy f32 %bits
    return f32 %value
}
function @signed_middle(%p : ptr) -> i16 [no_inline=yes] {
  block ^entry:
    %word = load i64 %p
    %middle = binary shr i64 %word, 8
    %value = convert trunc i16 i64 %middle
    return i16 %value
}
function @double_high(%p : ptr) -> f64 [no_inline=yes] {
  block ^entry:
    %word = load i128 %p
    %high = binary ushr i128 %word, 64
    %bits = convert trunc i64 i128 %high
    %value = copy f64 %bits
    return f64 %value
}
function @snapshot(%p : ptr) -> f32 [no_inline=yes] {
  block ^entry:
    %word = load i64 %p
    store i64 0, %p
    %high = binary ushr i64 %word, 32
    %bits = convert trunc u32 i64 %high
    %value = copy f32 %bits
    return f32 %value
}
function @volatile_word(%p : ptr) -> f32 [no_inline=yes] {
  block ^entry:
    %word = load volatile i64 %p
    %high = binary ushr i64 %word, 32
    %bits = convert trunc u32 i64 %high
    %value = copy f32 %bits
    return f32 %value
}
function @atomic_word(%p : ptr) -> f32 [no_inline=yes] {
  block ^entry:
    %word = atomic_load i64 %p, 0
    %high = binary ushr i64 %word, 32
    %bits = convert trunc u32 i64 %high
    %value = copy f32 %bits
    return f32 %value
}
function @overwrite(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    store i64 0, %p
    return void
}
function @snapshot_call(%p : ptr) -> f32 [no_inline=yes] {
  block ^entry:
    %word = load i64 %p
    call void @overwrite(%p)
    %high = binary ushr i64 %word, 32
    %bits = convert trunc u32 i64 %high
    %value = copy f32 %bits
    return f32 %value
}
function @shared_word(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %word = load i64 %p
    %high = binary ushr i64 %word, 32
    %bits = convert trunc u32 i64 %high
    %value = convert zext i64 u32 %bits
    %combined = binary xor i64 %word, %value
    return i64 %combined
}
function @unaligned_bits(%p : ptr) -> u16 [no_inline=yes] {
  block ^entry:
    %word = load i64 %p
    %middle = binary ushr i64 %word, 9
    %value = convert trunc u16 i64 %middle
    return u16 %value
}
function @numeric_float(%p : ptr) -> f32 [no_inline=yes] {
  block ^entry:
    %word = load i64 %p
    %value = convert sitofp f32 i64 %word
    return f32 %value
}
function @typed_alias(%p : ptr, %q : ptr, %replacement : i64) -> u32 [no_inline=yes] {
  block ^entry:
    %first = load typed i64 %p
    %a = binary ushr i64 %first, 32
    %aa = convert trunc u32 i64 %a
    %af = copy f32 %aa
    store typed i64 %replacement, %q
    %second = load typed i64 %p
    %b = binary ushr i64 %second, 32
    %bb = convert trunc u32 i64 %b
    %bf = copy f32 %bb
    %abits = copy u32 %af
    %bbits = copy u32 %bf
    %result = binary xor u32 %abits, %bbits
    return u32 %result
}
function @main() -> i32 [role=entry] {
  slot $cell : obj<16x16>
  block ^entry:
    %p = addr $cell
    store i64 0x80000000a1b2c3d4, %p
    %high = call f32 @float_high(%p)
    %high_bits = copy u32 %high
    %a = cmp eq u32 %high_bits, 0x80000000
    %low = call f32 @float_low(%p)
    %low_bits = copy u32 %low
    %b = cmp eq u32 %low_bits, 0xa1b2c3d4
    %middle = call i16 @signed_middle(%p)
    %c = cmp eq i16 %middle, -19773
    %unaligned = call u16 @unaligned_bits(%p)
    %d = cmp eq u16 %unaligned, 0xd961
    %shared = call i64 @shared_word(%p)
    %e = cmp eq i64 %shared, 0x8000000021b2c3d4
    %volatile = call f32 @volatile_word(%p)
    %volatile_bits = copy u32 %volatile
    %f = cmp eq u32 %volatile_bits, 0x80000000
    %atomic = call f32 @atomic_word(%p)
    %atomic_bits = copy u32 %atomic
    %l = cmp eq u32 %atomic_bits, 0x80000000
    %call_saved = call f32 @snapshot_call(%p)
    %call_bits = copy u32 %call_saved
    %m = cmp eq u32 %call_bits, 0x80000000
    store i64 0x80000000a1b2c3d4, %p
    %saved = call f32 @snapshot(%p)
    %saved_bits = copy u32 %saved
    %g = cmp eq u32 %saved_bits, 0x80000000
    %zero = load i64 %p
    %h = cmp eq i64 %zero, 0
    store i128 0x3ff40000000000000000000000000000, %p
    %double = call f64 @double_high(%p)
    %double_bits = copy i64 %double
    %i = cmp eq i64 %double_bits, 0x3ff4000000000000
    store i64 37, %p
    %number = call f32 @numeric_float(%p)
    %j = cmp eq f32 %number, 37f
    store i64 0x80000000a1b2c3d4, %p
    %aliased = call u32 @typed_alias(%p, %p, 0x3f800000a1b2c3d4)
    %k = cmp eq u32 %aliased, 0xbf800000
    %ab = binary and i64 %a, %b
    %cd = binary and i64 %c, %d
    %ef = binary and i64 %e, %f
    %gh = binary and i64 %g, %h
    %ij = binary and i64 %i, %j
    %abcd = binary and i64 %ab, %cd
    %efgh = binary and i64 %ef, %gh
    %first = binary and i64 %abcd, %efgh
    %ok = binary and i64 %first, %ij
    %all = binary and i64 %ok, %k
    %lm = binary and i64 %l, %m
    %complete = binary and i64 %all, %lm
    branch %complete, ^passed, ^failed
  block ^passed:
    return i32 0
  block ^failed:
    return i32 1
}
