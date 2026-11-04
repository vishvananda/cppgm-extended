# Same-width integer expressions wrap at each operation; shared values and
# ordered floating arithmetic retain every observable result.
function @affine_i8(%x : i8) -> i8 [binding=strong] {
  block ^entry:
    %v0 = binary mul i8 %x, 101
    %v1 = binary add i8 %v0, 37
    %v2 = binary mul i8 %v1, 103
    %v3 = binary sub i8 %v2, 19
    %v4 = binary mul i8 %v3, 107
    %v5 = binary add i8 %v4, 43
    %v6 = binary mul i8 %v5, 109
    return i8 %v6
}

function @affine_u8(%x : u8) -> u8 [binding=strong] {
  block ^entry:
    %v0 = binary mul u8 %x, 101
    %v1 = binary add u8 %v0, 37
    %v2 = binary mul u8 %v1, 103
    %v3 = binary sub u8 %v2, 19
    %v4 = binary mul u8 %v3, 107
    %v5 = binary add u8 %v4, 43
    %v6 = binary mul u8 %v5, 109
    return u8 %v6
}

function @affine_i16(%x : i16) -> i16 [binding=strong] {
  block ^entry:
    %v0 = binary mul i16 %x, 101
    %v1 = binary add i16 %v0, 37
    %v2 = binary mul i16 %v1, 103
    %v3 = binary sub i16 %v2, 19
    %v4 = binary mul i16 %v3, 107
    %v5 = binary add i16 %v4, 43
    %v6 = binary mul i16 %v5, 109
    return i16 %v6
}

function @affine_u16(%x : u16) -> u16 [binding=strong] {
  block ^entry:
    %v0 = binary mul u16 %x, 101
    %v1 = binary add u16 %v0, 37
    %v2 = binary mul u16 %v1, 103
    %v3 = binary sub u16 %v2, 19
    %v4 = binary mul u16 %v3, 107
    %v5 = binary add u16 %v4, 43
    %v6 = binary mul u16 %v5, 109
    return u16 %v6
}

function @affine_i32(%x : i32) -> i32 [binding=strong] {
  block ^entry:
    %v0 = binary mul i32 %x, 101
    %v1 = binary add i32 %v0, 37
    %v2 = binary mul i32 %v1, 103
    %v3 = binary sub i32 %v2, 19
    %v4 = binary mul i32 %v3, 107
    %v5 = binary add i32 %v4, 43
    %v6 = binary mul i32 %v5, 109
    return i32 %v6
}

function @affine_u32(%x : u32) -> u32 [binding=strong] {
  block ^entry:
    %v0 = binary mul u32 %x, 101
    %v1 = binary add u32 %v0, 37
    %v2 = binary mul u32 %v1, 103
    %v3 = binary sub u32 %v2, 19
    %v4 = binary mul u32 %v3, 107
    %v5 = binary add u32 %v4, 43
    %v6 = binary mul u32 %v5, 109
    return u32 %v6
}

function @affine_i64(%x : i64) -> i64 [binding=strong] {
  block ^entry:
    %v0 = binary mul i64 %x, 101
    %v1 = binary add i64 %v0, 37
    %v2 = binary mul i64 %v1, 103
    %v3 = binary sub i64 %v2, 19
    %v4 = binary mul i64 %v3, 107
    %v5 = binary add i64 %v4, 43
    %v6 = binary mul i64 %v5, 109
    return i64 %v6
}

function @affine_i128(%x : i128) -> i128 [binding=strong] {
  block ^entry:
    %v0 = binary mul i128 %x, 101
    %v1 = binary add i128 %v0, 37
    %v2 = binary mul i128 %v1, 103
    %v3 = binary sub i128 %v2, 19
    %v4 = binary mul i128 %v3, 107
    %v5 = binary add i128 %v4, 43
    %v6 = binary mul i128 %v5, 109
    return i128 %v6
}

function @shared(%x : i32) -> i32 [binding=strong] {
  block ^entry:
    %p = binary mul i32 %x, 101
    %q = binary add i32 %p, 37
    %r = binary mul i32 %q, 103
    %s = binary xor i32 %r, %p
    return i32 %s
}

function @ordered(%x : f64) -> f64 [binding=strong] {
  block ^entry:
    %p = binary mul f64 %x, 1.5
    %q = binary add f64 %p, -2.75
    %r = binary mul f64 %q, 3.25
    return f64 %r
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %r0 = call i8 @affine_i8(-127)
    %bad0 = cmp ne i8 %r0, 12
    %errors0 = binary or i64 0, %bad0
    %r1 = call u8 @affine_u8(255)
    %bad1 = cmp ne u8 %r1, 114
    %errors1 = binary or i64 %errors0, %bad1
    %r2 = call i16 @affine_i16(-32767)
    %bad2 = cmp ne i16 %r2, -15732
    %errors2 = binary or i64 %errors1, %bad2
    %r3 = call u16 @affine_u16(65535)
    %bad3 = cmp ne u16 %r3, 36466
    %errors3 = binary or i64 %errors2, %bad3
    %r4 = call i32 @affine_i32(-2147483647)
    %bad4 = cmp ne i32 %r4, -1981922676
    %errors4 = binary or i64 %errors3, %bad4
    %r5 = call u32 @affine_u32(4294967295)
    %bad5 = cmp ne u32 %r5, 4217867890
    %errors5 = binary or i64 %errors4, %bad5
    %r6 = call i64 @affine_i64(-9223372036854775807)
    %bad6 = cmp ne i64 %r6, -9223372036689214836
    %errors6 = binary or i64 %errors5, %bad6
    %r7 = call i128 @affine_i128(2596148429267413814265248164610065)
    %bad7 = cmp ne i128 %r7, -110292173720567541071430537775021825188
    %errors7 = binary or i64 %errors6, %bad7
    %shared = call i32 @shared(-2147483647)
    %bad_shared = cmp ne i32 %shared, 14307
    %errors_shared = binary or i64 %errors7, %bad_shared
    %ordered = call f64 @ordered(0.1)
    %bad_ordered = cmp ne f64 %ordered, -8.450000000000001
    %errors = binary or i64 %errors_shared, %bad_ordered
    return i64 %errors
}
