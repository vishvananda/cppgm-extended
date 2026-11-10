# Adjacent floating stores in lanes: lanes of one operation, adjacent loads
# and gathered values, and four f32 lanes.
function @pair(%p : ptr, %k : f64) -> void [binding=strong] {
  block ^entry:
    %a = index i8 %p, 16
    %b = index i8 %p, 24
    %c = index i8 %p, 32
    %d = index i8 %p, 40
    %w = load typed f64 %a
    %x = load typed f64 %b
    %m = binary mul f64 %w, %k
    %n = binary mul f64 %x, %k
    %y = load typed f64 %c
    %z = load typed f64 %d
    %u = binary add f64 %m, %y
    %v = binary add f64 %n, %z
    store typed f64 %u, %p
    %q = index i8 %p, 8
    store typed f64 %v, %q
    return void
}

function @copy_pair(%this : ptr) -> void [binding=strong] {
  block ^entry:
    %a = index i8 %this, 16
    %x = load typed f64 %a
    %b = index i8 %this, 24
    %y = load typed f64 %b
    %c = index i8 %this, 0
    %d = index i8 %this, 8
    store typed f64 %x, %c
    store typed f64 %y, %d
    return void
}

function @four(%this : ptr) -> void [binding=strong] {
  block ^entry:
    %a = index i8 %this, 16
    %b = index i8 %this, 20
    %c = index i8 %this, 24
    %d = index i8 %this, 28
    %w = load typed f32 %a
    %x = load typed f32 %b
    %y = load typed f32 %c
    %z = load typed f32 %d
    %e = index i8 %this, 0
    %f = index i8 %this, 4
    %g = index i8 %this, 8
    %h = index i8 %this, 12
    %i = load typed f32 %e
    %j = load typed f32 %f
    %k = load typed f32 %g
    %l = load typed f32 %h
    %m = binary mul f32 %i, %w
    %n = binary mul f32 %j, %x
    %o = binary mul f32 %k, %y
    %p = binary mul f32 %l, %z
    store typed f32 %m, %e
    store typed f32 %n, %f
    store typed f32 %o, %g
    store typed f32 %p, %h
    return void
}

# A load between the stores may read the first of them: they stay.
function @kept(%a : ptr, %b : ptr, %k : f64) -> f64 [binding=strong] {
  block ^entry:
    %x = load typed f64 %a
    %m = binary mul f64 %x, %k
    store typed f64 %m, %a
    %c = load typed f64 %b
    %y = index i8 %a, 8
    %z = load typed f64 %y
    %n = binary mul f64 %z, %k
    store typed f64 %n, %y
    return f64 %c
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
