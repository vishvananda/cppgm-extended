target wasm32 1
function @single(%value : obj<4x4> [shape=i32x1]) -> obj<4x4> [result_shape=i32x1] {
  block ^entry:
    return obj<4x4> %value
}
function @floating(%value : obj<8x8> [shape=f64x1]) -> obj<8x8> [result_shape=f64x1] {
  block ^entry:
    return obj<8x8> %value
}
function @pair(%value : obj<8x4> [shape=i32x2]) -> obj<8x4> [result_shape=i32x2] {
  block ^entry:
    return obj<8x4> %value
}
function @empty(%value : obj<1x1> [shape=empty]) -> obj<1x1> [result_shape=empty] {
  block ^entry:
    return obj<1x1> %value
}
