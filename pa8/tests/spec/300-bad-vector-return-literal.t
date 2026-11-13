# A v128 value is a temporary or the literal 0.
function @f() -> v128 {
  block ^entry:
    return v128 5
}
