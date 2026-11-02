# The init function calls a constructor; once it is inlined, its stores of
# constants into @point become @point's data and the final pass folds
# @norm's loads of it.
global @point : obj<16x8> [binding=internal] = {
  zero 16
}

function @construct(%p : ptr, %x : f64, %y : f64) -> void {
  block ^entry:
    store f64 %x, %p
    %q = index i8 %p, 8
    store f64 %y, %q
    return void
}

function @__cppgm_init() -> void [role=init, binding=internal] {
  block ^entry:
    %p = addr @point
    call void @construct(%p, 3, 4)
    return void
}

function @norm() -> f64 [no_inline=yes] {
  block ^entry:
    %p = addr @point
    %x = load f64 %p
    %q = index i8 %p, 8
    %y = load f64 %q
    %xx = binary mul f64 %x, %x
    %yy = binary mul f64 %y, %y
    %s = binary add f64 %xx, %yy
    return f64 %s
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %n = call f64 @norm()
    %bad = cmp ne f64 %n, 25
    %r = convert zext i32 i1 %bad
    return i32 %r
}
