target wasm32 1
declare function @take(%tag : i32) -> void [arity=variadic]
function @bad(%object : obj<4x4> [shape=i32x1]) -> void {
  block ^entry:
    call void @take(1, %object) [argument_shape=1:i32x1, argument_shape=1:mixed]
    return void
}
