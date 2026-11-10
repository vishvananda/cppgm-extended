# The init function's stores of constants into internal globals that start
# zero become their data: @two_pi (2 * readonly @pi), both fields of @body
# (the second from a load of @two_pi the init stored just before) and
# @after_call (stored after a call, which reads nothing it stored).
# @later stays dynamic (@set writes it too), as does @shown (external).
# main's loads of the static globals read their literals.
global @pi : f64 [storage=readonly, binding=internal] = 3.25
global @two_pi : f64 [binding=internal] = zero
global @body : obj<16x8> [binding=internal] = {
  zero 16
}
global @later : f64 [binding=internal] = zero
global @after_call : f64 [binding=internal] = zero
global @shown : i32 = zero
global @count : i32 [binding=internal] = zero

function @tick() -> void [no_inline=yes] {
  block ^entry:
    %c = load i32 @count
    %d = binary add i32 %c, 1
    store i32 %d, @count
    return void
}

function @set(%x : f64) -> void [no_inline=yes] {
  block ^entry:
    store f64 %x, @later
    return void
}

function @__cppgm_init() -> void [role=init, binding=internal] {
  block ^entry:
    %p = load f64 @pi
    %t = binary mul f64 %p, 2
    store f64 %t, @two_pi
    %b = addr @body
    store f64 %t, %b
    %r = load f64 @two_pi
    %h = binary mul f64 %r, 0.5
    %m = index i8 %b, 8
    store f64 %h, %m
    store f64 1.5, @later
    call void @tick()
    store f64 2.5, @after_call
    store i32 7, @shown
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %before = load f64 @later
    call void @set(4)
    %now = load f64 @later
    %t = load f64 @two_pi
    %b = addr @body
    %b0 = load f64 %b
    %m = index i8 %b, 8
    %b1 = load f64 %m
    %a = load f64 @after_call
    %s = load i32 @shown
    %c = load i32 @count
    %s1 = binary add f64 %t, %b0
    %s2 = binary add f64 %s1, %b1
    %s3 = binary add f64 %s2, %a
    %s4 = binary add f64 %s3, %before
    %s5 = binary add f64 %s4, %now
    %bad_f = cmp ne f64 %s5, 24.25
    %bad_s = cmp ne i32 %s, 7
    %bad_c = cmp ne i32 %c, 1
    %x = convert zext i32 i1 %bad_f
    %y = convert zext i32 i1 %bad_s
    %z = convert zext i32 i1 %bad_c
    %xy = binary or i32 %x, %y
    %all = binary or i32 %xy, %z
    return i32 %all
}
