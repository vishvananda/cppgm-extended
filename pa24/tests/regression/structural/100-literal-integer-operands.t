function @masked(%value : i64) -> i64 [unwind=no] {
  block ^entry:
    %ones = binary shl i32 1, 4
    %low = binary sub i32 %ones, 1
    %wide = convert sext i64 i32 %low
    %mask = unary bitnot i64 %wide
    %result = binary and i64 %value, %mask
    return i64 %result
}

function @unsigned_shift() -> i64 [unwind=no] {
  block ^entry:
    %shift = binary ushr i32 -1, 4
    %result = convert zext i64 i32 %shift
    return i64 %result
}

function @signed_shift() -> i64 [unwind=no] {
  block ^entry:
    %shift = binary shr u8 128, 2
    %result = convert sext i64 u8 %shift
    return i64 %result
}

function @narrow() -> i64 [unwind=no] {
  block ^entry:
    %sum = binary add i8 127, 1
    %result = convert sext i64 i8 %sum
    return i64 %result
}

function @truncated() -> i64 [unwind=no] {
  block ^entry:
    %sum = binary add i64 255, 3
    %lo = convert trunc u8 i64 %sum
    %neg = unary neg u8 %lo
    %result = convert zext i64 u8 %neg
    return i64 %result
}

function @boolean() -> i64 [unwind=no] {
  block ^entry:
    %result = convert sext i64 i1 1
    return i64 %result
}

function @out_of_range() -> i32 [unwind=no] {
  block ^entry:
    %result = binary shl i32 1, 32
    return i32 %result
}

function @main() -> i32 [role=entry, unwind=no] {
  block ^entry:
    %a = call i64 @masked(12345)
    %bad_a = cmp ne i64 %a, 12336
    %b = call i64 @unsigned_shift()
    %bad_b = cmp ne i64 %b, 268435455
    %c = call i64 @signed_shift()
    %bad_c = cmp ne i64 %c, -32
    %d = call i64 @narrow()
    %bad_d = cmp ne i64 %d, -128
    %e = call i64 @truncated()
    %bad_e = cmp ne i64 %e, 254
    %f = call i64 @boolean()
    %bad_f = cmp ne i64 %f, -1
    %bad0 = binary or i1 %bad_a, %bad_b
    %bad1 = binary or i1 %bad_c, %bad_d
    %bad2 = binary or i1 %bad_e, %bad_f
    %bad3 = binary or i1 %bad0, %bad1
    %bad = binary or i1 %bad3, %bad2
    %exit = convert zext i32 i1 %bad
    return i32 %exit
}
