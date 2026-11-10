# A value compared unequal to itself is a NaN test: ucomisd sets the parity
# flag only for an unordered pair, so the branch is one jp, with no jne. The
# NaN side is rare: placement puts it out of line and the loop falls
# through to its body.

function @clean(%p : ptr, %n : i64) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^next: %j]
    %s = phi f64 [^entry: 0.0, ^next: %t]
    %off = binary mul i64 %i, 8
    %a = binary add ptr %p, %off
    %v = load f64 %a
    %nan = cmp ne f64 %v, %v
    branch %nan, ^skip, ^add
  block ^add:
    %s1 = binary add f64 %s, %v
    jump ^next
  block ^skip:
    %s2 = binary add f64 %s, 1000.0
    jump ^next
  block ^next:
    %t = phi f64 [^add: %s1, ^skip: %s2]
    %j = binary add i64 %i, 1
    %more = cmp lt i64 %j, %n
    branch %more, ^head, ^out
  block ^out:
    return f64 %t
}

function @main() -> i32 [role=entry] {
  slot $data : obj<32x8>

  block ^entry:
    %buf = addr $data
    %zero = binary sub f64 0.0, 0.0
    %q = binary div f64 %zero, %zero
    store f64 1.5, %buf
    %b1 = binary add ptr %buf, 8
    store f64 %q, %b1
    %b2 = binary add ptr %buf, 16
    store f64 2.5, %b2
    %b3 = binary add ptr %buf, 24
    store f64 3.0, %b3
    %r = call f64 @clean(%buf, 4)
    %w = convert fptosi i32 f64 %r
    %m = binary mod i32 %w, 251
    return i32 %m
}
