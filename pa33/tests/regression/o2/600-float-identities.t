function @kept(%x : f32, %y : f64) -> f64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary mul f32 1f, %x
    %b = binary mul f32 %a, 1f
    %c = binary div f32 %b, 1f
    %d = binary sub f32 %c, 0f
    %e = binary add f32 %d, -0f
    %f = binary add f32 -0f, %e
    %g = convert fpext f64 f32 %f
    %h = binary mul f64 1, %y
    %i = binary div f64 %h, 1
    %j = binary sub f64 %i, 0
    %k = binary add f64 %j, -0
    %l = binary add f64 %g, %k
    return f64 %l
}

function @not_kept(%x : f32) -> f32 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary add f32 %x, 0f
    %b = binary sub f32 %a, -0f
    %c = binary sub f32 0f, %b
    %d = binary div f32 1f, %c
    %e = binary mul f32 %d, 2f
    return f32 %e
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
