declare function @take(%tag : i32) -> void [arity=variadic]
function @bad() -> void {
  block ^entry:
    call void @take(1, 2) [argument_shape=1:i32x1]
    return void
}
