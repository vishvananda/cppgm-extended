# A counter that steps by a value the loop does not compute has products of its
# own: here 16 times the counter, and 16 times the counter plus an invariant.
# Both step by 16 times the stride. The call behind the test for not a number
# is a rare path and does not stop it.
declare function @note(%x : f64) -> f64 [linkage=c, binding=strong, object=note]

function @butterfly(%a : ptr, %m : i64, %half : i64, %n : i64) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %k = phi i64 [^entry: 0, ^next: %step]
    %t = phi f64 [^entry: 0, ^next: %u]
    %far = binary add i64 %k, %half
    %fo = binary mul i64 %far, 16
    %p = index i8 %a, %fo
    %x = load f64 %p
    %no = binary mul i64 %k, 16
    %q = index i8 %a, %no
    %y = load f64 %q
    %z = binary mul f64 %x, %y
    %nan = cmp ne f64 %z, %z
    branch %nan, ^rare, ^next
  block ^rare:
    %r = call f64 @note(%z)
    jump ^next
  block ^next:
    %v = phi f64 [^body: %z, ^rare: %r]
    store f64 %v, %q
    %u = binary add f64 %t, %v
    %step = binary add i64 %k, %m
    %c = cmp lt i64 %step, %n
    branch %c, ^body, ^done
  block ^done:
    return f64 %u
}

# A literal step with a factor no address scale gives, the same way.
function @pairs(%a : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: 0, ^body: %sum]
    %o = binary mul i64 %i, 16
    %p = index i8 %a, %o
    %v = load i64 %p
    %sum = binary add i64 %s, %v
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %sum
}

# Refused: an address scale gives a product by 8, and the call is on the usual
# path.
declare function @use(%x : i64) -> void [linkage=c, binding=strong]

function @scaled(%a : ptr, %m : i64, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %k = phi i64 [^entry: 0, ^body: %step]
    %s = phi i64 [^entry: 0, ^body: %sum]
    %o = binary mul i64 %k, 8
    %p = index i8 %a, %o
    %v = load i64 %p
    %sum = binary add i64 %s, %v
    %step = binary add i64 %k, %m
    %c = cmp lt i64 %step, %n
    branch %c, ^body, ^done
  block ^done:
    return i64 %sum
}

function @called(%m : i64, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %k = phi i64 [^entry: 0, ^body: %step]
    %o = binary mul i64 %k, 24
    call void @use(%o)
    %step = binary add i64 %k, %m
    %c = cmp lt i64 %step, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
