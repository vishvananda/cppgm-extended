# Word instructions establish their result's upper bits; signed wide reads
# still need the original sign, and zero/variable counts keep their semantics.
function @logical(%x : u32) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary ushr u32 %x, 7
    %out = convert zext i64 u32 %r
    return i64 %out
}

function @arithmetic(%x : i32) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary shr i32 %x, 7
    %out = convert sext i64 i32 %r
    return i64 %out
}

function @signed_return(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %r = binary shr i32 %x, 7
    return i32 %r
}

function @unsigned_arithmetic(%x : u32) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary shr u32 %x, 7
    %out = convert zext i64 u32 %r
    return i64 %out
}

function @bitwise(%x : u32, %y : u32) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary xor u32 %x, %y
    %out = convert zext i64 u32 %r
    return i64 %out
}

function @zero_count(%x : u32) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary ushr u32 %x, 0
    %out = convert zext i64 u32 %r
    return i64 %out
}

function @variable(%x : u32, %k : u32) -> i64 [no_inline=yes] {
  block ^entry:
    %r = binary ushr u32 %x, %k
    %out = convert zext i64 u32 %r
    return i64 %out
}

function @hash(%p : ptr) -> u32 [no_inline=yes] {
  block ^entry:
    %bits = copy i64 %p
    %word = convert trunc u32 i64 %bits
    %a = binary ushr u32 %word, 4
    %b = binary ushr u32 %word, 9
    %r = binary xor u32 %a, %b
    return u32 %r
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %a = call i64 @logical(4294967295)
    %b = call i64 @arithmetic(-1)
    %c = call i32 @signed_return(-2147483648)
    %d = call i64 @unsigned_arithmetic(4294967295)
    %e = call i64 @bitwise(2147483648, 2147483647)
    %f = call i64 @zero_count(4294967295)
    %g = call i64 @variable(4294967295, 0)
    %h = call i64 @variable(4294967295, 31)
    %p = copy ptr 1311768467463790320
    %i = call u32 @hash(%p)
    %ab = cmp ne i64 %a, 33554431
    %bb = cmp ne i64 %b, -1
    %cb = cmp ne i32 %c, -16777216
    %db = cmp ne i64 %d, 4294967295
    %eb = cmp ne i64 %e, 4294967295
    %fb = cmp ne i64 %f, 4294967295
    %gb = cmp ne i64 %g, 4294967295
    %hb = cmp ne i64 %h, 1
    %ib = cmp ne u32 %i, 166105984
    %r1 = binary or i64 %ab, %bb
    %r2 = binary or i64 %cb, %db
    %r3 = binary or i64 %eb, %fb
    %r4 = binary or i64 %gb, %hb
    %r5 = binary or i64 %r1, %r2
    %r6 = binary or i64 %r3, %r4
    %r7 = binary or i64 %r5, %r6
    %bad = binary or i64 %r7, %ib
    return i64 %bad
}
