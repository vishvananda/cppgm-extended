# A small callee goes inline whatever its instructions are, once each has a
# charge: an atomic load, x87 arithmetic and a long clear have one. A
# read-modify-write atomic has none, but a callee no larger than its call
# with the atomic counted as a few moves, @bump, goes inline too.
function @bump(%a : ptr) -> i64 [binding=internal] {
  block ^entry:
    %r = atomic_add_fetch i64 %a, 1, 5
    return i64 %r
}
function @peek(%a : ptr) -> i64 [binding=internal] {
  block ^entry:
    %r = atomic_load i64 %a, 5
    return i64 %r
}
function @scale(%a : f80, %b : f80) -> f80 [binding=internal] {
  block ^entry:
    %m = binary mul f80 %a, %b
    %r = binary add f80 %m, %a
    return f80 %r
}
function @clear(%p : ptr) -> void [binding=internal] {
  block ^entry:
    zeroinit 256x8 %p
    return void
}
function @count(%a : ptr) -> i64 {
  block ^entry:
    %x = call i64 @bump(%a)
    %y = call i64 @peek(%a)
    %z = binary add i64 %x, %y
    return i64 %z
}
function @count_again(%a : ptr, %b : ptr) -> i64 {
  block ^entry:
    %x = call i64 @bump(%b)
    %y = call i64 @peek(%a)
    %z = binary sub i64 %x, %y
    return i64 %z
}
function @product(%a : f80, %b : f80) -> f80 {
  block ^entry:
    %x = call f80 @scale(%a, %b)
    %y = call f80 @scale(%x, %b)
    return f80 %y
}
function @reset(%p : ptr, %q : ptr) -> void {
  block ^entry:
    call void @clear(%p)
    call void @clear(%q)
    return void
}
