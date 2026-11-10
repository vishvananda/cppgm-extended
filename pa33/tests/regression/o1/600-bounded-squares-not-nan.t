# A branch on `x*x + y*y <= 4` proves on its true edge both squares in
# [0, 4]: a square is at least 0 or a NaN, and the sum is no NaN. So x and
# y are finite, and x*x - y*y is no NaN: @escape's NaN test (`d != d`, as
# complex multiplication makes it) is false and its call goes. @negated
# leaves `ugt` on its false edge, which is `le`. @product bounds x*x + x*y,
# and x*y may be negative: its test stays.
function @slow(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    return f64 %x
}

function @escape(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %xx = binary mul f64 %x, %x
    %yy = binary mul f64 %y, %y
    %s = binary add f64 %xx, %yy
    %in = cmp le f64 %s, 4
    branch %in, ^inside, ^outside

  block ^inside:
    %d = binary sub f64 %xx, %yy
    %nan = cmp ne f64 %d, %d
    branch %nan, ^fix, ^done

  block ^fix:
    %f = call f64 @slow(%d)
    jump ^done

  block ^done:
    %r = phi f64 [^inside: %d, ^fix: %f]
    return f64 %r

  block ^outside:
    return f64 %s
}

function @negated(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %xx = binary mul f64 %x, %x
    %yy = binary mul f64 %y, %y
    %s = binary add f64 %xx, %yy
    %out = cmp ugt f64 %s, 4
    branch %out, ^outside, ^inside

  block ^inside:
    %d = binary sub f64 %xx, %yy
    %nan = cmp ne f64 %d, %d
    branch %nan, ^fix, ^done

  block ^fix:
    %f = call f64 @slow(%d)
    jump ^done

  block ^done:
    %r = phi f64 [^inside: %d, ^fix: %f]
    return f64 %r

  block ^outside:
    return f64 %s
}

function @product(%x : f64, %y : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %xx = binary mul f64 %x, %x
    %xy = binary mul f64 %x, %y
    %s = binary add f64 %xx, %xy
    %in = cmp le f64 %s, 4
    branch %in, ^inside, ^outside

  block ^inside:
    %d = binary sub f64 %xx, %xy
    %nan = cmp ne f64 %d, %d
    branch %nan, ^fix, ^done

  block ^fix:
    %f = call f64 @slow(%d)
    jump ^done

  block ^done:
    %r = phi f64 [^inside: %d, ^fix: %f]
    return f64 %r

  block ^outside:
    return f64 %s
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call f64 @escape(1.5, 0.5)
    %b = call f64 @negated(0.5, 1.5)
    %c = call f64 @product(1, 1)
    %ab = binary add f64 %a, %b
    %abc = binary add f64 %ab, %c
    %bad = cmp ne f64 %abc, 0
    %r = convert zext i32 i1 %bad
    return i32 %r
}
