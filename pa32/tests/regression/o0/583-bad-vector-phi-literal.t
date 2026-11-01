# A v128 value is a temporary or the literal 0.
function @f(%c : i32) -> v128 {
  block ^entry:
    branch %c, ^a, ^b
  block ^a:
    jump ^j
  block ^b:
    jump ^j
  block ^j:
    %p = phi v128 [^a: 5, ^b: 0]
    return v128 %p
}
