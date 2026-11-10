# Ordinary masks may discard high read bits; signed promotions must never widen an access.
function @byte(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %v = load i64 %p
    %masked = binary and i64 %v, 255
    return i64 %masked
}
function @half(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %v = load i64 %p
    %masked = binary and i64 %v, 65535
    return i64 %masked
}
function @word(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %v = load i64 %p
    %masked = binary and i64 %v, 4294967295
    return i64 %masked
}
function @signed_word(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %v = load i32 %p
    %wide = convert sext i64 i32 %v
    %masked = binary and i64 %wide, 4294967295
    return i64 %masked
}
function @signed_byte(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %v = load i8 %p
    %wide = convert sext i64 i8 %v
    %masked = binary and i64 %wide, 255
    return i64 %masked
}
function @no_widen_byte(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %v = load i8 %p
    %wide = convert sext i64 i8 %v
    %masked = binary and i64 %wide, 65535
    return i64 %masked
}
function @no_widen_half(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %v = load i16 %p
    %wide = convert sext i64 i16 %v
    %masked = binary and i64 %wide, 4294967295
    return i64 %masked
}
function @main() -> i64 [role=entry] {
  slot $data : i64
  block ^entry:
    %p = addr $data
    store i64 -1, %p
    %v0 = call i64 @byte(%p)
    %b0 = cmp ne i64 %v0, 255
    %v1 = call i64 @half(%p)
    %b1 = cmp ne i64 %v1, 65535
    %v2 = call i64 @word(%p)
    %b2 = cmp ne i64 %v2, 4294967295
    %v3 = call i64 @signed_word(%p)
    %b3 = cmp ne i64 %v3, 4294967295
    store i64 255, %p
    %v4 = call i64 @signed_byte(%p)
    %b4 = cmp ne i64 %v4, 255
    %v5 = call i64 @no_widen_byte(%p)
    %b5 = cmp ne i64 %v5, 65535
    store i64 65535, %p
    %v6 = call i64 @no_widen_half(%p)
    %b6 = cmp ne i64 %v6, 4294967295
    %r1 = binary or i64 %b0, %b1
    %r2 = binary or i64 %r1, %b2
    %r3 = binary or i64 %r2, %b3
    %r4 = binary or i64 %r3, %b4
    %r5 = binary or i64 %r4, %b5
    %r6 = binary or i64 %r5, %b6
    return i64 %r6
}
