# Floating add and mul commute for value numbering: the second product is the
# first, and a literal goes right.
function @twice(%x : f64, %y : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary mul f64 %x, %y
    %b = binary mul f64 %y, %x
    %c = binary add f64 %a, %b
    %d = binary add f64 2, %c
    return f64 %d
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %r = call f64 @twice(3, 4)
    %i = convert fptosi i32 f64 %r
    %c = binary sub i32 %i, 26
    return i32 %c
}
