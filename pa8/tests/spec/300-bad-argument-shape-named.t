target wasm32 1
declare function @take(%object : obj<4x4> [shape=i32x1]) -> void [arity=variadic]
function @bad(%object : obj<4x4> [shape=i32x1]) -> void {
  block ^entry:
    call void @take(%object) [argument_shape=0:i32x1]
    return void
}
