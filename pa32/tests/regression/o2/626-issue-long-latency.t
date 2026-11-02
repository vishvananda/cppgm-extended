# A square root or a division takes many cycles, and what reads it waits
# behind it. An instruction about to read one that nothing has read yet first
# brings forward the next ones of the block with their pure operands, at most
# four unread at once: the second and third pairs' roots and quotients start
# before the first quotient's reader. Nothing moves past the call, and a root
# of what an integer division gives stays: the division may trap. The second
# root's operand reads %cd twice, once through %cd2: %cd moves first.
declare function @note(%x : f64) -> void [linkage=c, binding=strong]

function @pairs(%a : f64, %b : f64, %c : f64, %d : f64, %e : f64, %f : f64, %n : i64, %m : i64) -> f64 [no_inline=yes] {
  block ^entry:
    %ab = binary sub f64 %a, %b
    %ab2 = binary mul f64 %ab, %ab
    %r1 = unary sqrt f64 %ab2
    %k1 = binary div f64 1, %r1
    %u1 = binary mul f64 %k1, %ab
    %cd = binary sub f64 %c, %d
    %cd2 = binary mul f64 %cd, %cd
    %cs = binary add f64 %cd2, %cd
    %r2 = unary sqrt f64 %cs
    %k2 = binary div f64 1, %r2
    %u2 = binary mul f64 %k2, %cd
    %ef = binary sub f64 %e, %f
    %ef2 = binary mul f64 %ef, %ef
    %r3 = unary sqrt f64 %ef2
    %k3 = binary div f64 1, %r3
    %u3 = binary mul f64 %k3, %ef
    %s1 = binary add f64 %u1, %u2
    %s2 = binary add f64 %s1, %u3
    call void @note(%s2)
    %ae = binary sub f64 %a, %e
    %r4 = unary sqrt f64 %ae
    %v4 = binary mul f64 %r4, %s2
    %q = binary div i64 %n, %m
    %qf = convert sitofp f64 i64 %q
    %bf = binary sub f64 %b, %f
    %bq = binary add f64 %bf, %qf
    %r5 = unary sqrt f64 %bq
    %t = binary add f64 %v4, %r5
    %w = binary add f64 %t, %f
    return f64 %w
}

# Six roots each read once: at most four start ahead of their readers.
function @six(%a : f64, %b : f64, %c : f64, %d : f64, %e : f64, %f : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %r1 = unary sqrt f64 %a
    %y1 = binary mul f64 %r1, %a
    %r2 = unary sqrt f64 %b
    %y2 = binary mul f64 %r2, %b
    %s2 = binary add f64 %y1, %y2
    %r3 = unary sqrt f64 %c
    %y3 = binary mul f64 %r3, %c
    %s3 = binary add f64 %s2, %y3
    %r4 = unary sqrt f64 %d
    %y4 = binary mul f64 %r4, %d
    %s4 = binary add f64 %s3, %y4
    %r5 = unary sqrt f64 %e
    %y5 = binary mul f64 %r5, %e
    %s5 = binary add f64 %s4, %y5
    %r6 = unary sqrt f64 %f
    %y6 = binary mul f64 %r6, %f
    %s6 = binary add f64 %s5, %y6
    return f64 %s6
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
