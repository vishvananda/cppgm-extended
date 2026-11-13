# A block copy or zeroing goes through an XMM register that holds nothing in
# the function: one of xmm2-xmm7 that no instruction names and no call takes
# an argument in (its moves need no REX prefix), else xmm15. `@copy` and
# `@clear` name no XMM register. `@forward` names none either, but its eight
# arguments stay in xmm0-xmm7 for the call to `@sum`, so its copy goes
# through xmm15 and the sum is still 36.
function @sum(%a : f64, %b : f64, %c : f64, %d : f64, %e : f64, %f : f64, %g : f64, %h : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %t1 = binary add f64 %a, %b
    %t2 = binary add f64 %t1, %c
    %t3 = binary add f64 %t2, %d
    %t4 = binary add f64 %t3, %e
    %t5 = binary add f64 %t4, %f
    %t6 = binary add f64 %t5, %g
    %t7 = binary add f64 %t6, %h
    return f64 %t7
}

function @forward(%from : ptr, %to : ptr, %a : f64, %b : f64, %c : f64, %d : f64, %e : f64, %f : f64, %g : f64, %h : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    copyobj 32x8 %from, %to
    %r = call f64 @sum(%a, %b, %c, %d, %e, %f, %g, %h)
    return f64 %r
}

function @copy(%from : ptr, %to : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    copyobj 32x8 %from, %to
    return void
}

function @clear(%p : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    zeroinit 32x8 %p
    return void
}

function @main() -> i32 [role=entry] {
  slot $a : obj<32x8>
  slot $b : obj<32x8>
  slot $c : obj<32x8>

  block ^entry:
    %pa = addr $a
    store i64 1, %pa
    %pa1 = index i64 %pa, 1
    store i64 2, %pa1
    %pa2 = index i64 %pa, 2
    store i64 3, %pa2
    %pa3 = index i64 %pa, 3
    store i64 4, %pa3
    %pb = addr $b
    call void @copy(%pa, %pb)
    %pc = addr $c
    %r = call f64 @forward(%pb, %pc, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0)
    call void @clear(%pa)
    %c0 = load i64 %pc
    %pc3 = index i64 %pc, 3
    %c3 = load i64 %pc3
    %a0 = load i64 %pa
    %a3 = load i64 %pa3
    %ri = convert fptosi i32 f64 %r
    %s1 = binary add i64 %c0, %c3
    %s2 = binary add i64 %s1, %a0
    %s3 = binary add i64 %s2, %a3
    %si = convert trunc i32 i64 %s3
    %t = binary add i32 %ri, %si
    return i32 %t
}
