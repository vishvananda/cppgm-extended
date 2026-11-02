function @payload() -> f64 [no_inline=yes] {
  block ^entry:
    %bits = copy i64 0x7ff8000000000012
    %value = copy f64 %bits
    return f64 %value
}

function @main() -> i32 {
  block ^entry:
    %negative_zero = copy f64 -0.0
    %float_value = copy f32 1.5f
    %a = copy i64 %negative_zero
    %b = copy f64 %a
    %c = copy u32 %float_value
    %d = copy f32 %c
    %x = cmp eq i64 %a, -9223372036854775808
    %y = cmp eq u32 %c, 1069547520
    %z = cmp eq f64 %b, 0.0
    %w = cmp eq f32 %d, 1.5f
    %p = binary and i1 %x, %y
    %q = binary and i1 %z, %w
    %ok = binary and i1 %p, %q
    %nan = call f64 @payload()
    %payload = copy i64 %nan
    %same = cmp eq i64 %payload, 0x7ff8000000000012
    %all = binary and i1 %ok, %same
    %r = select i32 %all, 0, 1
    return i32 %r
}
