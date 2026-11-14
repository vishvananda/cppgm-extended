# The init function stores @x's address into @at and then a constant into
# @x. @poke writes @x through the pointer it loads from @at, so @x stays
# dynamic and main's load of @x after the call is not folded; @at, whose
# only writer is the init function, becomes data holding the address, and
# @poke's load of it is then that address.
global @x : f64 [binding=internal] = zero
global @at : ptr [binding=internal] = zero

function @poke(%v : f64) -> void [no_inline=yes] {
  block ^entry:
    %p = load ptr @at
    store f64 %v, %p
    return void
}

function @__cppgm_init() -> void [role=init, binding=internal] {
  block ^entry:
    %a = addr @x
    store ptr %a, @at
    store f64 1.5, @x
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %before = load f64 @x
    call void @poke(5)
    %after = load f64 @x
    %s = binary add f64 %before, %after
    %bad = cmp ne f64 %s, 6.5
    %r = convert zext i32 i1 %bad
    return i32 %r
}
