# A consumer may establish the upper bits; other readers still need them.
function @sum_bytes(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %a = load i8 %p
    %ax = convert sext u32 i8 %a
    %ar = binary shl u32 %ax, 13
    %aw = convert zext i64 u32 %ar
    %p1 = index i8 %p, 1
    %b = load i8 %p1
    %bx = convert sext u32 i8 %b
    %br = binary shl u32 %bx, 11
    %bw = convert zext i64 u32 %br
    %p2 = index i8 %p, 2
    %c = load i8 %p2
    %cx = convert sext u32 i8 %c
    %cr = binary add u32 %cx, 91
    %cw = convert zext i64 u32 %cr
    %p3 = index i8 %p, 3
    %d = load i8 %p3
    %dx = convert sext u32 i8 %d
    %dr = binary mul u32 %dx, 7
    %dw = convert zext i64 u32 %dr
    %ab = binary add i64 %aw, %bw
    %cd = binary add i64 %cw, %dw
    %sum = binary add i64 %ab, %cd
    return i64 %sum
}

function @right(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %n = load i8 %p
    %x = convert sext u32 i8 %n
    %r = binary ushr u32 %x, 7
    %out = convert zext i64 u32 %r
    return i64 %out
}

function @wide(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %n = load i8 %p
    %x = convert sext i64 i8 %n
    %r = binary shl i64 %x, 13
    return i64 %r
}

function @zero(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %n = load i8 %p
    %x = convert sext u32 i8 %n
    %r = binary shl u32 %x, 0
    %out = convert zext i64 u32 %r
    return i64 %out
}

function @identity(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %n = load i8 %p
    %x = convert sext u32 i8 %n
    %r = binary mul u32 %x, 1
    %out = convert zext i64 u32 %r
    return i64 %out
}

function @variable(%p : ptr, %shift : u32) -> i64 [no_inline=yes] {
  block ^entry:
    %n = load i8 %p
    %x = convert sext u32 i8 %n
    %r = binary shl u32 %x, %shift
    %out = convert zext i64 u32 %r
    return i64 %out
}

function @main() -> i64 [role=entry] {
  slot $bytes : obj<4x1>

  block ^entry:
    %p = index i8 $bytes, 0
    %p1 = index i8 $bytes, 1
    %p2 = index i8 $bytes, 2
    %p3 = index i8 $bytes, 3
    store i8 -1, %p
    store i8 -128, %p1
    store i8 -64, %p2
    store i8 -1, %p3
    %sum = call i64 @sum_bytes(%p)
    %right = call i64 @right(%p)
    %wide = call i64 @wide(%p)
    %zero = call i64 @zero(%p)
    %identity = call i64 @identity(%p)
    %variable0 = call i64 @variable(%p, 0)
    %variable1 = call i64 @variable(%p, 1)
    %variable31 = call i64 @variable(%p, 31)
    %sum_bad = cmp ne i64 %sum, 12884631572
    %right_bad = cmp ne i64 %right, 33554431
    %wide_bad = cmp ne i64 %wide, -8192
    %zero_bad = cmp ne i64 %zero, 4294967295
    %identity_bad = cmp ne i64 %identity, 4294967295
    %variable0_bad = cmp ne i64 %variable0, 4294967295
    %variable1_bad = cmp ne i64 %variable1, 4294967294
    %variable31_bad = cmp ne i64 %variable31, 2147483648
    %a = convert zext i64 u8 %sum_bad
    %b = convert zext i64 u8 %right_bad
    %c = convert zext i64 u8 %wide_bad
    %d = convert zext i64 u8 %zero_bad
    %e = convert zext i64 u8 %identity_bad
    %f = convert zext i64 u8 %variable0_bad
    %g = convert zext i64 u8 %variable1_bad
    %h = convert zext i64 u8 %variable31_bad
    %ab = binary or i64 %a, %b
    %cd = binary or i64 %c, %d
    %ef = binary or i64 %e, %f
    %gh = binary or i64 %g, %h
    %abcd = binary or i64 %ab, %cd
    %efgh = binary or i64 %ef, %gh
    %bad = binary or i64 %abcd, %efgh
    return i64 %bad
}
