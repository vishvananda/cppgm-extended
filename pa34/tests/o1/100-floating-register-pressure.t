# Fourteen floating values live at once fit the XMM registers, and a result may
# take the register of an operand that dies with it. A subtraction or division
# keeps its operand order, and a call clobbers every XMM register.
function @opaque(%x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %y = binary mul f64 %x, 0.25
    return f64 %y
}
function @pressure(%p : ptr) -> f64 [no_inline=yes] {
  block ^entry:
    %at0 = index f64 %p, 0
    %x0 = load f64 %at0
    %at1 = index f64 %p, 1
    %x1 = load f64 %at1
    %at2 = index f64 %p, 2
    %x2 = load f64 %at2
    %at3 = index f64 %p, 3
    %x3 = load f64 %at3
    %at4 = index f64 %p, 4
    %x4 = load f64 %at4
    %at5 = index f64 %p, 5
    %x5 = load f64 %at5
    %at6 = index f64 %p, 6
    %x6 = load f64 %at6
    %at7 = index f64 %p, 7
    %x7 = load f64 %at7
    %at8 = index f64 %p, 8
    %x8 = load f64 %at8
    %at9 = index f64 %p, 9
    %x9 = load f64 %at9
    %at10 = index f64 %p, 10
    %x10 = load f64 %at10
    %at11 = index f64 %p, 11
    %x11 = load f64 %at11
    %at12 = index f64 %p, 12
    %x12 = load f64 %at12
    %at13 = index f64 %p, 13
    %x13 = load f64 %at13
    %r12 = binary sub f64 %x13, %x12
    %r11 = binary sub f64 %r12, %x11
    %r10 = binary sub f64 %r11, %x10
    %r9 = binary sub f64 %r10, %x9
    %r8 = binary sub f64 %r9, %x8
    %r7 = binary sub f64 %r8, %x7
    %r6 = binary sub f64 %r7, %x6
    %r5 = binary sub f64 %r6, %x5
    %r4 = binary sub f64 %r5, %x4
    %r3 = binary sub f64 %r4, %x3
    %r2 = binary sub f64 %r3, %x2
    %r1 = binary sub f64 %r2, %x1
    %r0 = binary sub f64 %r1, %x0
    %s1 = binary div f64 %x0, %x1
    %s2 = binary div f64 %s1, %x2
    %s3 = binary div f64 %s2, %x3
    %s4 = binary div f64 %s3, %x4
    %s5 = binary div f64 %s4, %x5
    %s6 = binary div f64 %s5, %x6
    %s7 = binary div f64 %s6, %x7
    %s8 = binary div f64 %s7, %x8
    %s9 = binary div f64 %s8, %x9
    %s10 = binary div f64 %s9, %x10
    %s11 = binary div f64 %s10, %x11
    %s12 = binary div f64 %s11, %x12
    %scaled = binary mul f64 %s12, 302231454903657293676544.0
    %total = binary add f64 %r0, %scaled
    return f64 %total
}
function @across_call(%p : ptr) -> f64 [no_inline=yes] {
  block ^entry:
    %at0 = index f64 %p, 0
    %x0 = load f64 %at0
    %at1 = index f64 %p, 1
    %x1 = load f64 %at1
    %at2 = index f64 %p, 2
    %x2 = load f64 %at2
    %at3 = index f64 %p, 3
    %x3 = load f64 %at3
    %at4 = index f64 %p, 4
    %x4 = load f64 %at4
    %at5 = index f64 %p, 5
    %x5 = load f64 %at5
    %at6 = index f64 %p, 6
    %x6 = load f64 %at6
    %at7 = index f64 %p, 7
    %x7 = load f64 %at7
    %at8 = index f64 %p, 8
    %x8 = load f64 %at8
    %at9 = index f64 %p, 9
    %x9 = load f64 %at9
    %at10 = index f64 %p, 10
    %x10 = load f64 %at10
    %at11 = index f64 %p, 11
    %x11 = load f64 %at11
    %at12 = index f64 %p, 12
    %x12 = load f64 %at12
    %at13 = index f64 %p, 13
    %x13 = load f64 %at13
    %called = call f64 @opaque(%x0)
    %r12 = binary sub f64 %x13, %x12
    %r11 = binary sub f64 %r12, %x11
    %r10 = binary sub f64 %r11, %x10
    %r9 = binary sub f64 %r10, %x9
    %r8 = binary sub f64 %r9, %x8
    %r7 = binary sub f64 %r8, %x7
    %r6 = binary sub f64 %r7, %x6
    %r5 = binary sub f64 %r6, %x5
    %r4 = binary sub f64 %r5, %x4
    %r3 = binary sub f64 %r4, %x3
    %r2 = binary sub f64 %r3, %x2
    %r1 = binary sub f64 %r2, %x1
    %r0 = binary sub f64 %r1, %x0
    %s1 = binary div f64 %x0, %x1
    %s2 = binary div f64 %s1, %x2
    %s3 = binary div f64 %s2, %x3
    %s4 = binary div f64 %s3, %x4
    %s5 = binary div f64 %s4, %x5
    %s6 = binary div f64 %s5, %x6
    %s7 = binary div f64 %s6, %x7
    %s8 = binary div f64 %s7, %x8
    %s9 = binary div f64 %s8, %x9
    %s10 = binary div f64 %s9, %x10
    %s11 = binary div f64 %s10, %x11
    %s12 = binary div f64 %s11, %x12
    %scaled = binary mul f64 %s12, 302231454903657293676544.0
    %total = binary add f64 %r0, %scaled
    %result = binary add f64 %total, %called
    return f64 %result
}
function @in_place(%p : ptr, %x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = load f64 %p
    %b = binary sub f64 %a, %x
    %c = binary div f64 %b, %x
    %d = binary mul f64 %c, %x
    return f64 %d
}
function @keeps_numerator(%a : f64, %b : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %c = binary add f64 %b, 1.0
    %d = binary div f64 %a, %c
    %e = binary add f64 %d, %a
    return f64 %e
}
function @keeps_minuend(%a : f32, %b : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %c = binary add f32 %b, 1.0
    %d = binary sub f32 %a, %c
    %e = binary mul f32 %d, %a
    return f32 %e
}
function @main() -> i64 [role=entry] {
  slot $cells : obj<112x8>
  block ^entry:
    %cells = addr $cells
    jump ^fill

  block ^fill:
    %i = phi i64 [^entry: 0, ^fill: %next]
    %value = phi f64 [^entry: 1.0, ^fill: %doubled]
    %at = index f64 %cells, %i
    store f64 %value, %at
    %doubled = binary mul f64 %value, 2.0
    %next = binary add i64 %i, 1
    %more = cmp ult i64 %next, 14
    branch %more, ^fill, ^check

  block ^check:
    %a = call f64 @pressure(%cells)
    %bad_a = cmp ne f64 %a, 2.0
    %b = call f64 @across_call(%cells)
    %bad_b = cmp ne f64 %b, 2.25
    %c = call f64 @in_place(%cells, 4.0)
    %bad_c = cmp ne f64 %c, -3.0
    %d = call f64 @keeps_numerator(6.0, 2.0)
    %bad_d = cmp ne f64 %d, 8.0
    %e = call f32 @keeps_minuend(5.0, 2.0)
    %bad_e = cmp ne f32 %e, 10.0
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %abcd = binary or i64 %ab, %cd
    %failed = binary or i64 %abcd, %bad_e
    %status = convert trunc i32 i64 %failed
    return i64 %status
}
