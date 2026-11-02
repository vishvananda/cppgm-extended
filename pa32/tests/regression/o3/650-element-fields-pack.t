# The two lanes of a body's speed update pack although the other body's mass,
# reached through another element of the array, sits between them: its field
# is not theirs.
function @kick(%p : ptr, %i : i32, %j : i32, %dx : f64, %dy : f64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = convert sext i64 i32 %i
    %b = convert sext i64 i32 %j
    %x = binary mul i64 %a, 56
    %y = binary mul i64 %b, 56
    %e = index i8 %p, %x
    %f = index i8 %p, %y
    %mass = index i8 %f, 48
    %vx = index i8 %e, 24
    %vy = index i8 %e, 32
    %m1 = load typed f64 %mass
    %p1 = binary mul f64 %dx, %m1
    %s1 = load typed f64 %vx
    %t1 = binary sub f64 %s1, %p1
    store typed f64 %t1, %vx
    %m2 = load typed f64 %mass
    %p2 = binary mul f64 %dy, %m2
    %s2 = load typed f64 %vy
    %t2 = binary sub f64 %s2, %p2
    store typed f64 %t2, %vy
    return void
}

global @bodies : obj<112x8> = { zero 112 }

function @main() -> i32 [role=entry] {
  block ^entry:
    %c = index i8 @bodies, 104
    store typed f64 2, %c
    call void @kick(@bodies, 0, 1, 3, 4)
    %q = index i8 @bodies, 24
    %r = index i8 @bodies, 32
    %a = load typed f64 %q
    %b = load typed f64 %r
    %s = binary add f64 %a, %b
    %t = convert fptosi i32 f64 %s
    return i32 %t
}
