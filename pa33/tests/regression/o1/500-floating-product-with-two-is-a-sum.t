# A floating product with the literal two is the sum of the value with itself,
# in either operand order and in both widths: the result is the same for every
# value, the largest ones included. A product with another literal stays.
global @largest : i64 = 9218868437227405311

function @twice(%x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = binary mul f64 %x, 2.0
    return f64 %r
}

function @twice_left(%x : f32) -> f32 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = binary mul f32 2.0, %x
    return f32 %r
}

function @thrice(%x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = binary mul f64 %x, 3.0
    return f64 %r
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %a = call f64 @twice(1.5)
    %a_bad = cmp ne f64 %a, 3.0
    %b = call f32 @twice_left(-0.25)
    %b_bad = cmp ne f32 %b, -0.5
    %c = call f64 @thrice(1.5)
    %c_bad = cmp ne f64 %c, 4.5
    %at = addr @largest
    %largest = load f64 %at
    %d = call f64 @twice(%largest)
    %d_bits = copy i64 %d
    %d_bad = cmp ne i64 %d_bits, 9218868437227405312
    %bad0 = binary or i64 %a_bad, %b_bad
    %bad1 = binary or i64 %c_bad, %d_bad
    %bad = binary or i64 %bad0, %bad1
    return i64 %bad
}
