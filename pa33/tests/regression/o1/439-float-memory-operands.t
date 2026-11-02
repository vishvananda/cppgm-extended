# A float load whose only reader comes next is that reader's memory operand: an
# add or multiply on either side, the right of a subtract or divide, and the
# operand a compare reads second. On the left of a subtract, the load stays.
function @add_left(%p : ptr, %x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %v = load typed f64 %p
    %r = binary add f64 %v, %x
    return f64 %r
}

function @sub_right(%p : ptr, %x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %v = load typed f64 %p
    %r = binary sub f64 %x, %v
    return f64 %r
}

function @sub_left(%p : ptr, %x : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %v = load typed f64 %p
    %r = binary sub f64 %v, %x
    return f64 %r
}

function @divide(%p : ptr, %x : f32) -> f32 [no_inline=yes] {
  block ^entry:
    %v = load typed f32 %p
    %r = binary div f32 %x, %v
    return f32 %r
}

function @both(%p : ptr, %q : ptr) -> f64 [no_inline=yes] {
  block ^entry:
    %a = load typed f64 %p
    %b = load typed f64 %q
    %r = binary mul f64 %a, %b
    return f64 %r
}

function @less(%p : ptr, %x : f64) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load typed f64 %p
    %c = cmp lt f64 %v, %x
    branch %c, ^yes, ^no
  block ^yes:
    return i32 1
  block ^no:
    return i32 2
}

function @more(%p : ptr, %x : f64) -> i32 [no_inline=yes] {
  block ^entry:
    %v = load typed f64 %p
    %c = cmp gt f64 %x, %v
    branch %c, ^yes, ^no
  block ^yes:
    return i32 3
  block ^no:
    return i32 4
}

function @main() -> i32 [role=entry] {
  slot $a : obj<16x8>
  block ^entry:
    %p = addr $a
    %q = index i8 %p, 8
    store typed f64 6, %p
    store typed f64 2, %q
    %b = call f64 @add_left(%p, 1)
    %c = call f64 @sub_right(%p, 10)
    %d = call f64 @sub_left(%q, 10)
    %e = call f64 @both(%p, %q)
    %f = binary add f64 %b, %c
    %g = binary add f64 %d, %e
    %h = binary add f64 %f, %g
    %i = convert fptosi i32 f64 %h
    %j = call i32 @less(%p, 7)
    %k = call i32 @more(%p, 5)
    %l = binary mul i32 %j, 10
    %m = binary add i32 %l, %k
    %n = binary add i32 %m, %i
    return i32 %n
}
