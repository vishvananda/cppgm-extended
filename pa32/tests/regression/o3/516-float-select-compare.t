# A floating compare moves beside the select that reads it, past the add, so
# the back end can fuse it into the cmov.

function @count(%p : ptr, %e : ptr, %k : f32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %q = phi ptr [^entry: %p, ^body: %next]
    %n = phi i64 [^entry: 0, ^body: %m]
    %v = load f32 %q
    %c = cmp ge f32 %k, %v
    %up = binary add i64 %n, 1
    %m = select i64 %c, %up, %n
    %next = index i8 %q, 8
    %more = cmp ne ptr %e, %next
    branch %more, ^body, ^done
  block ^done:
    return i64 %m
}

# A floating equality takes two conditions and stays where it is.
function @equal(%a : f64, %b : f64, %x : i64, %y : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %c = cmp eq f64 %a, %b
    %up = binary add i64 %x, 1
    %m = select i64 %c, %up, %y
    return i64 %m
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
