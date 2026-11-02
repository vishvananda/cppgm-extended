# A square root keeps the upper bits of its destination, and so would wait for
# whatever last wrote that register: the operand moves there first. Here it is
# read again, so the root cannot take its register.
function @both(%x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = unary sqrt f64 %x
    %s = binary add f64 %r, %x
    return f64 %s
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %r = call f64 @both(4)
    %i = convert fptosi i32 f64 %r
    %c = binary sub i32 %i, 6
    return i32 %c
}
