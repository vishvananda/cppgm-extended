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
    %r = select i32 %ok, 0, 1
    return i32 %r
}
