function @immediate(%x : i32) -> i32 {
  block ^entry:
    %c = cmp eq i32 %x, 7
    %v = convert trunc i32 i64 %c
    return i32 %v
}
function @large_immediate(%x : i64) -> i32 {
  block ^entry:
    %c = cmp eq i64 %x, 4294967297
    %v = convert trunc i32 i64 %c
    return i32 %v
}
function @boolean_branch(%p : ptr) -> i32 {
  block ^entry:
    %v = load i1 %p
    branch %v, ^yes, ^no
  block ^yes:
    return i32 17
  block ^no:
    return i32 29
}
function @volatile_boolean(%p : ptr) -> i32 {
  block ^entry:
    %v = load volatile i1 %p
    branch %v, ^yes, ^no
  block ^yes:
    return i32 17
  block ^no:
    return i32 29
}
function @floating_identity(%x : f64) -> f64 {
  block ^entry:
    return f64 %x
}
function @mixed(%f : f32, %a : i64, %b : i64) -> i64 {
  block ^entry:
    %x = binary add i64 %a, %b
    %y = binary xor i64 %a, %b
    %z = binary mul i64 %x, %y
    %i = convert fptosi i64 f32 %f
    %r = binary add i64 %z, %i
    return i64 %r
}
function @extended_to_double(%v : f80) -> f64 {
  block ^entry:
    %r = convert fptrunc f64 f80 %v
    return f64 %r
}
function @main() -> i32 [role=entry] {
  slot $flag : i1
  block ^entry:
    store i1 1, $flag
    %ptr = addr $flag
    %a = call i32 @immediate(7)
    %b = call i32 @large_immediate(4294967297)
    %c = call i32 @boolean_branch(%ptr)
    %d = call i32 @volatile_boolean(%ptr)
    %f = call f64 @floating_identity(1.5)
    %g = call i64 @mixed(2.0, 3, 5)
    %extended = call f64 @extended_to_double(1.5L)
    %extended_bad = cmp ne f64 %extended, 1.5
    %ba0 = cmp ne i32 %a, 1
    %ba = binary or i64 %ba0, %extended_bad
    %bb = cmp ne i32 %b, 1
    %bc = cmp ne i32 %c, 17
    %bd = cmp ne i32 %d, 17
    %bf = cmp ne f64 %f, 1.5
    %bg = cmp ne i64 %g, 50
    %p = binary or i64 %ba, %bb
    %q = binary or i64 %bc, %bd
    %r = binary or i64 %bf, %bg
    %s = binary or i64 %p, %q
    %t = binary or i64 %s, %r
    %exit = convert trunc i32 i64 %t
    return i32 %exit
}
