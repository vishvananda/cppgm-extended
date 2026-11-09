function @top32(%x : i32) -> i32 [no_inline=yes, unwind=no] {
  block ^entry:
    %masked = binary and i32 %x, -16777216
    %result = binary shr exact i32 %masked, 24
    return i32 %result
}
function @top64(%x : i64) -> i64 [no_inline=yes, unwind=no] {
  block ^entry:
    %masked = binary and i64 %x, -65536
    %result = binary ushr exact i64 %masked, 16
    return i64 %result
}
function @keep_sign_mask(%x : i32) -> i32 [no_inline=yes, unwind=no] {
  block ^entry:
    %masked = binary and i32 %x, 2147418112
    %result = binary shr i32 %masked, 16
    return i32 %result
}
function @keep_middle_bit(%x : i64) -> i64 [no_inline=yes, unwind=no] {
  block ^entry:
    %masked = binary and i64 %x, -17
    %result = binary ushr i64 %masked, 3
    return i64 %result
}
function @shared_mask(%x : i32) -> i32 [no_inline=yes, unwind=no] {
  block ^entry:
    %masked = binary and i32 %x, -256
    %shifted = binary shr i32 %masked, 8
    %result = binary xor i32 %masked, %shifted
    return i32 %result
}
function @main() -> i32 {
  block ^entry:
    %a = call i32 @top32(-2147483647)
    %a_ok = cmp eq i32 %a, -128
    %b = call i64 @top64(-1)
    %b_ok = cmp eq i64 %b, 281474976710655
    %c = call i32 @keep_sign_mask(-1)
    %c_ok = cmp eq i32 %c, 32767
    %d = call i64 @keep_middle_bit(31)
    %d_ok = cmp eq i64 %d, 1
    %e = call i32 @shared_mask(-255)
    %e_ok = cmp eq i32 %e, 255
    %ab = binary and i64 %a_ok, %b_ok
    %cd = binary and i64 %c_ok, %d_ok
    %abcd = binary and i64 %ab, %cd
    %all = binary and i64 %abcd, %e_ok
    branch %all, ^pass, ^fail
  block ^pass:
    return i32 0
  block ^fail:
    return i32 1
}
