# A floating division by a power of two is a multiplication by its exact
# reciprocal. 3 has no exact reciprocal, and neither has a value so large that
# its reciprocal is subnormal.
function @half(%x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %h = binary div f64 %x, 2
    return f64 %h
}

function @quarter(%x : f32) -> f32 [binding=strong, no_inline=yes] {
  block ^entry:
    %h = binary div f32 %x, -4f
    return f32 %h
}

function @third(%x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %h = binary div f64 %x, 3
    return f64 %h
}

function @huge(%x : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %h = binary div f64 %x, 0x1p+1023
    return f64 %h
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call f64 @half(8)
    %b = call f32 @quarter(8f)
    %c = call f64 @third(9)
    %d = call f64 @huge(0)
    %e = convert fpext f64 f32 %b
    %f = binary add f64 %a, %e
    %g = binary add f64 %f, %c
    %h = binary add f64 %g, %d
    %i = convert fptosi i32 f64 %h
    %j = binary sub i32 %i, 5
    return i32 %j
}
