target wasm32 1
function @one(%x : obj<4x4> [shape=i32x1]) -> i32 {
  block ^entry:
    return i32 0
}
function @other(%x : obj<4x4> [shape=mixed]) -> i32 {
  block ^entry:
    return i32 0
}
equivalent_entry @one = @other
