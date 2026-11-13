# The square root of an f64 and of an f32, of a parameter and of a literal,
# and of a negative operand, which is a NaN.
function @root64(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r = unary sqrt f64 %x
    return f64 %r
}

function @root32(%x : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %r = unary sqrt f32 %x
    return f32 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call f64 @root64(2.25)
    %a_ok = cmp eq f64 %a, 1.5
    branch %a_ok, ^b, ^fail

  block ^b:
    %b = call f32 @root32(6.25f)
    %b_ok = cmp eq f32 %b, 2.5f
    branch %b_ok, ^c, ^fail

  block ^c:
    %c = call f64 @root64(-1.0)
    %c_nan = cmp ne f64 %c, %c
    branch %c_nan, ^d, ^fail

  block ^d:
    %d = unary sqrt f64 16.0
    %d_ok = cmp eq f64 %d, 4.0
    branch %d_ok, ^pass, ^fail

  block ^pass:
    return i32 0

  block ^fail:
    return i32 1
}
