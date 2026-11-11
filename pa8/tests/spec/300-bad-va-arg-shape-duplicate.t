target wasm32 1
function @bad(%list : ptr) -> void {
  block ^entry:
    %object = va_arg obj<4x4> %list [shape=i32x1, shape=mixed]
    return void
}
