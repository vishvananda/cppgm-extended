function @main() -> i64 {
  block ^entry:
    %a = unary sqrt f64 16.0
    %b = unary sqrt f32 6.25
    %ok1 = cmp eq f64 %a, 4.0
    %ok2 = cmp eq f32 %b, 2.5
    %s = binary add i64 %ok1, %ok2
    return i64 %s
}
