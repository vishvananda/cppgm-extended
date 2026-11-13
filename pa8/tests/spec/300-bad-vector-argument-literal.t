# A v128 value is a temporary or the literal 0.
function @g(%a : v128) -> void {
  block ^entry:
    return void
}
function @f() -> void {
  block ^entry:
    call void @g(5)
    return void
}
