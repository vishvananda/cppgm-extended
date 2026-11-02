# The two halves of a butterfly: neither group of stores pays for its
# gathered pair and its loaded pair alone, both together do. The second
# group reuses the first one's vectors.
function @butterfly(%p : ptr, %q : ptr, %a : f64, %b : f64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %u0 = load typed f64 %p
    %pi = index i8 [projection=field] %p, 8
    %u1 = load typed f64 %pi
    %s0 = binary add f64 %a, %u0
    %s1 = binary add f64 %b, %u1
    store typed f64 %s0, %p
    store typed f64 %s1, %pi
    %d0 = binary sub f64 %u0, %a
    %d1 = binary sub f64 %u1, %b
    %qi = index i8 [projection=field] %q, 8
    store typed f64 %d0, %q
    store typed f64 %d1, %qi
    return void
}

function @main() -> i32 [role=entry] {
  slot $x : obj<32x8>

  block ^entry:
    %p = addr $x
    store f64 1.5, %p
    %p1 = index i8 %p, 8
    store f64 2.5, %p1
    %q = index i8 %p, 16
    call void @butterfly(%p, %q, 0.5, 4)
    %r0 = load f64 %p
    %r1 = load f64 %p1
    %r2 = load f64 %q
    %q1 = index i8 %p, 24
    %r3 = load f64 %q1
    %s = binary add f64 %r0, %r1
    %t = binary mul f64 %r2, 10
    %u = binary add f64 %s, %t
    %v = binary add f64 %u, %r3
    %bad = cmp ne f64 %v, 17
    %r = convert zext i32 i1 %bad
    return i32 %r
}
