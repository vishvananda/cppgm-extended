# A compare's flags must outlast everything between it and the select reading
# them. With every clamped value live to the end some spill, so the select's
# operand moves after the compare and its zero, displaced from the register by
# the 255 before it, is made again there: that zero must not be the xor idiom,
# which clears the flags the select reads. Each difference here is negative and
# clamps to zero; main returns how many do not.
function @clamp(%a : ptr, %b : ptr, %out : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %x0_at = index i32 [projection=array_element] %a, 0
    %x0 = load typed i32 %x0_at
    %y0_at = index i32 [projection=array_element] %b, 0
    %y0 = load typed i32 %y0_at
    %x1_at = index i32 [projection=array_element] %a, 1
    %x1 = load typed i32 %x1_at
    %y1_at = index i32 [projection=array_element] %b, 1
    %y1 = load typed i32 %y1_at
    %x2_at = index i32 [projection=array_element] %a, 2
    %x2 = load typed i32 %x2_at
    %y2_at = index i32 [projection=array_element] %b, 2
    %y2 = load typed i32 %y2_at
    %x3_at = index i32 [projection=array_element] %a, 3
    %x3 = load typed i32 %x3_at
    %y3_at = index i32 [projection=array_element] %b, 3
    %y3 = load typed i32 %y3_at
    %x4_at = index i32 [projection=array_element] %a, 4
    %x4 = load typed i32 %x4_at
    %y4_at = index i32 [projection=array_element] %b, 4
    %y4 = load typed i32 %y4_at
    %x5_at = index i32 [projection=array_element] %a, 5
    %x5 = load typed i32 %x5_at
    %y5_at = index i32 [projection=array_element] %b, 5
    %y5 = load typed i32 %y5_at
    %x6_at = index i32 [projection=array_element] %a, 6
    %x6 = load typed i32 %x6_at
    %y6_at = index i32 [projection=array_element] %b, 6
    %y6 = load typed i32 %y6_at
    %x7_at = index i32 [projection=array_element] %a, 7
    %x7 = load typed i32 %x7_at
    %y7_at = index i32 [projection=array_element] %b, 7
    %y7 = load typed i32 %y7_at
    %x8_at = index i32 [projection=array_element] %a, 8
    %x8 = load typed i32 %x8_at
    %y8_at = index i32 [projection=array_element] %b, 8
    %y8 = load typed i32 %y8_at
    %x9_at = index i32 [projection=array_element] %a, 9
    %x9 = load typed i32 %x9_at
    %y9_at = index i32 [projection=array_element] %b, 9
    %y9 = load typed i32 %y9_at
    %x10_at = index i32 [projection=array_element] %a, 10
    %x10 = load typed i32 %x10_at
    %y10_at = index i32 [projection=array_element] %b, 10
    %y10 = load typed i32 %y10_at
    %x11_at = index i32 [projection=array_element] %a, 11
    %x11 = load typed i32 %x11_at
    %y11_at = index i32 [projection=array_element] %b, 11
    %y11 = load typed i32 %y11_at
    %x12_at = index i32 [projection=array_element] %a, 12
    %x12 = load typed i32 %x12_at
    %y12_at = index i32 [projection=array_element] %b, 12
    %y12 = load typed i32 %y12_at
    %x13_at = index i32 [projection=array_element] %a, 13
    %x13 = load typed i32 %x13_at
    %y13_at = index i32 [projection=array_element] %b, 13
    %y13 = load typed i32 %y13_at
    %x14_at = index i32 [projection=array_element] %a, 14
    %x14 = load typed i32 %x14_at
    %y14_at = index i32 [projection=array_element] %b, 14
    %y14 = load typed i32 %y14_at
    %x15_at = index i32 [projection=array_element] %a, 15
    %x15 = load typed i32 %x15_at
    %y15_at = index i32 [projection=array_element] %b, 15
    %y15 = load typed i32 %y15_at
    %d0 = binary sub i32 %x0, %y0
    %n0 = cmp lt i32 %d0, 0
    %l0 = select i32 %n0, 0, %d0
    %h0 = cmp gt i32 %l0, 255
    %c0 = select i32 %h0, 255, %l0
    %d1 = binary sub i32 %x1, %y1
    %n1 = cmp lt i32 %d1, 0
    %l1 = select i32 %n1, 0, %d1
    %h1 = cmp gt i32 %l1, 255
    %c1 = select i32 %h1, 255, %l1
    %d2 = binary sub i32 %x2, %y2
    %n2 = cmp lt i32 %d2, 0
    %l2 = select i32 %n2, 0, %d2
    %h2 = cmp gt i32 %l2, 255
    %c2 = select i32 %h2, 255, %l2
    %d3 = binary sub i32 %x3, %y3
    %n3 = cmp lt i32 %d3, 0
    %l3 = select i32 %n3, 0, %d3
    %h3 = cmp gt i32 %l3, 255
    %c3 = select i32 %h3, 255, %l3
    %d4 = binary sub i32 %x4, %y4
    %n4 = cmp lt i32 %d4, 0
    %l4 = select i32 %n4, 0, %d4
    %h4 = cmp gt i32 %l4, 255
    %c4 = select i32 %h4, 255, %l4
    %d5 = binary sub i32 %x5, %y5
    %n5 = cmp lt i32 %d5, 0
    %l5 = select i32 %n5, 0, %d5
    %h5 = cmp gt i32 %l5, 255
    %c5 = select i32 %h5, 255, %l5
    %d6 = binary sub i32 %x6, %y6
    %n6 = cmp lt i32 %d6, 0
    %l6 = select i32 %n6, 0, %d6
    %h6 = cmp gt i32 %l6, 255
    %c6 = select i32 %h6, 255, %l6
    %d7 = binary sub i32 %x7, %y7
    %n7 = cmp lt i32 %d7, 0
    %l7 = select i32 %n7, 0, %d7
    %h7 = cmp gt i32 %l7, 255
    %c7 = select i32 %h7, 255, %l7
    %d8 = binary sub i32 %x8, %y8
    %n8 = cmp lt i32 %d8, 0
    %l8 = select i32 %n8, 0, %d8
    %h8 = cmp gt i32 %l8, 255
    %c8 = select i32 %h8, 255, %l8
    %d9 = binary sub i32 %x9, %y9
    %n9 = cmp lt i32 %d9, 0
    %l9 = select i32 %n9, 0, %d9
    %h9 = cmp gt i32 %l9, 255
    %c9 = select i32 %h9, 255, %l9
    %d10 = binary sub i32 %x10, %y10
    %n10 = cmp lt i32 %d10, 0
    %l10 = select i32 %n10, 0, %d10
    %h10 = cmp gt i32 %l10, 255
    %c10 = select i32 %h10, 255, %l10
    %d11 = binary sub i32 %x11, %y11
    %n11 = cmp lt i32 %d11, 0
    %l11 = select i32 %n11, 0, %d11
    %h11 = cmp gt i32 %l11, 255
    %c11 = select i32 %h11, 255, %l11
    %d12 = binary sub i32 %x12, %y12
    %n12 = cmp lt i32 %d12, 0
    %l12 = select i32 %n12, 0, %d12
    %h12 = cmp gt i32 %l12, 255
    %c12 = select i32 %h12, 255, %l12
    %d13 = binary sub i32 %x13, %y13
    %n13 = cmp lt i32 %d13, 0
    %l13 = select i32 %n13, 0, %d13
    %h13 = cmp gt i32 %l13, 255
    %c13 = select i32 %h13, 255, %l13
    %d14 = binary sub i32 %x14, %y14
    %n14 = cmp lt i32 %d14, 0
    %l14 = select i32 %n14, 0, %d14
    %h14 = cmp gt i32 %l14, 255
    %c14 = select i32 %h14, 255, %l14
    %d15 = binary sub i32 %x15, %y15
    %n15 = cmp lt i32 %d15, 0
    %l15 = select i32 %n15, 0, %d15
    %h15 = cmp gt i32 %l15, 255
    %c15 = select i32 %h15, 255, %l15
    %o0_at = index i32 [projection=array_element] %out, 0
    store typed i32 %c0, %o0_at
    %o1_at = index i32 [projection=array_element] %out, 1
    store typed i32 %c1, %o1_at
    %o2_at = index i32 [projection=array_element] %out, 2
    store typed i32 %c2, %o2_at
    %o3_at = index i32 [projection=array_element] %out, 3
    store typed i32 %c3, %o3_at
    %o4_at = index i32 [projection=array_element] %out, 4
    store typed i32 %c4, %o4_at
    %o5_at = index i32 [projection=array_element] %out, 5
    store typed i32 %c5, %o5_at
    %o6_at = index i32 [projection=array_element] %out, 6
    store typed i32 %c6, %o6_at
    %o7_at = index i32 [projection=array_element] %out, 7
    store typed i32 %c7, %o7_at
    %o8_at = index i32 [projection=array_element] %out, 8
    store typed i32 %c8, %o8_at
    %o9_at = index i32 [projection=array_element] %out, 9
    store typed i32 %c9, %o9_at
    %o10_at = index i32 [projection=array_element] %out, 10
    store typed i32 %c10, %o10_at
    %o11_at = index i32 [projection=array_element] %out, 11
    store typed i32 %c11, %o11_at
    %o12_at = index i32 [projection=array_element] %out, 12
    store typed i32 %c12, %o12_at
    %o13_at = index i32 [projection=array_element] %out, 13
    store typed i32 %c13, %o13_at
    %o14_at = index i32 [projection=array_element] %out, 14
    store typed i32 %c14, %o14_at
    %o15_at = index i32 [projection=array_element] %out, 15
    store typed i32 %c15, %o15_at
    %m0 = cmp ne i32 %c0, 0
    %w0 = convert zext i32 i1 %m0
    %s0 = binary add i32 0, %w0
    %m1 = cmp ne i32 %c1, 0
    %w1 = convert zext i32 i1 %m1
    %s1 = binary add i32 %s0, %w1
    %m2 = cmp ne i32 %c2, 0
    %w2 = convert zext i32 i1 %m2
    %s2 = binary add i32 %s1, %w2
    %m3 = cmp ne i32 %c3, 0
    %w3 = convert zext i32 i1 %m3
    %s3 = binary add i32 %s2, %w3
    %m4 = cmp ne i32 %c4, 0
    %w4 = convert zext i32 i1 %m4
    %s4 = binary add i32 %s3, %w4
    %m5 = cmp ne i32 %c5, 0
    %w5 = convert zext i32 i1 %m5
    %s5 = binary add i32 %s4, %w5
    %m6 = cmp ne i32 %c6, 0
    %w6 = convert zext i32 i1 %m6
    %s6 = binary add i32 %s5, %w6
    %m7 = cmp ne i32 %c7, 0
    %w7 = convert zext i32 i1 %m7
    %s7 = binary add i32 %s6, %w7
    %m8 = cmp ne i32 %c8, 0
    %w8 = convert zext i32 i1 %m8
    %s8 = binary add i32 %s7, %w8
    %m9 = cmp ne i32 %c9, 0
    %w9 = convert zext i32 i1 %m9
    %s9 = binary add i32 %s8, %w9
    %m10 = cmp ne i32 %c10, 0
    %w10 = convert zext i32 i1 %m10
    %s10 = binary add i32 %s9, %w10
    %m11 = cmp ne i32 %c11, 0
    %w11 = convert zext i32 i1 %m11
    %s11 = binary add i32 %s10, %w11
    %m12 = cmp ne i32 %c12, 0
    %w12 = convert zext i32 i1 %m12
    %s12 = binary add i32 %s11, %w12
    %m13 = cmp ne i32 %c13, 0
    %w13 = convert zext i32 i1 %m13
    %s13 = binary add i32 %s12, %w13
    %m14 = cmp ne i32 %c14, 0
    %w14 = convert zext i32 i1 %m14
    %s14 = binary add i32 %s13, %w14
    %m15 = cmp ne i32 %c15, 0
    %w15 = convert zext i32 i1 %m15
    %s15 = binary add i32 %s14, %w15
    return i32 %s15
}

function @main() -> i32 [role=entry] {
  slot $a : obj<64x4>
  slot $b : obj<64x4>
  slot $out : obj<64x4>

  block ^entry:
    %a0 = index i32 [projection=array_element] $a, 0
    store typed i32 1, %a0
    %b0 = index i32 [projection=array_element] $b, 0
    store typed i32 100, %b0
    %a1 = index i32 [projection=array_element] $a, 1
    store typed i32 2, %a1
    %b1 = index i32 [projection=array_element] $b, 1
    store typed i32 103, %b1
    %a2 = index i32 [projection=array_element] $a, 2
    store typed i32 3, %a2
    %b2 = index i32 [projection=array_element] $b, 2
    store typed i32 106, %b2
    %a3 = index i32 [projection=array_element] $a, 3
    store typed i32 4, %a3
    %b3 = index i32 [projection=array_element] $b, 3
    store typed i32 109, %b3
    %a4 = index i32 [projection=array_element] $a, 4
    store typed i32 5, %a4
    %b4 = index i32 [projection=array_element] $b, 4
    store typed i32 112, %b4
    %a5 = index i32 [projection=array_element] $a, 5
    store typed i32 6, %a5
    %b5 = index i32 [projection=array_element] $b, 5
    store typed i32 115, %b5
    %a6 = index i32 [projection=array_element] $a, 6
    store typed i32 7, %a6
    %b6 = index i32 [projection=array_element] $b, 6
    store typed i32 118, %b6
    %a7 = index i32 [projection=array_element] $a, 7
    store typed i32 8, %a7
    %b7 = index i32 [projection=array_element] $b, 7
    store typed i32 121, %b7
    %a8 = index i32 [projection=array_element] $a, 8
    store typed i32 9, %a8
    %b8 = index i32 [projection=array_element] $b, 8
    store typed i32 124, %b8
    %a9 = index i32 [projection=array_element] $a, 9
    store typed i32 10, %a9
    %b9 = index i32 [projection=array_element] $b, 9
    store typed i32 127, %b9
    %a10 = index i32 [projection=array_element] $a, 10
    store typed i32 11, %a10
    %b10 = index i32 [projection=array_element] $b, 10
    store typed i32 130, %b10
    %a11 = index i32 [projection=array_element] $a, 11
    store typed i32 12, %a11
    %b11 = index i32 [projection=array_element] $b, 11
    store typed i32 133, %b11
    %a12 = index i32 [projection=array_element] $a, 12
    store typed i32 13, %a12
    %b12 = index i32 [projection=array_element] $b, 12
    store typed i32 136, %b12
    %a13 = index i32 [projection=array_element] $a, 13
    store typed i32 14, %a13
    %b13 = index i32 [projection=array_element] $b, 13
    store typed i32 139, %b13
    %a14 = index i32 [projection=array_element] $a, 14
    store typed i32 15, %a14
    %b14 = index i32 [projection=array_element] $b, 14
    store typed i32 142, %b14
    %a15 = index i32 [projection=array_element] $a, 15
    store typed i32 16, %a15
    %b15 = index i32 [projection=array_element] $b, 15
    store typed i32 145, %b15
    %pa = addr $a
    %pb = addr $b
    %po = addr $out
    %wrong = call i32 @clamp(%pa, %pb, %po)
    return i32 %wrong
}
