target wasm32 1
declare function @single(%x : obj<4x4> [shape=i32x1]) -> obj<4x4> [result_shape=i32x1]
declare function @nested(%x : obj<8x8> [shape=f64x1]) -> obj<8x8> [result_shape=f64x1]
declare function @homogeneous(%x : obj<16x4> [shape=f32x4]) -> obj<16x4> [result_shape=f32x4]
declare function @heterogeneous(%x : obj<8x4> [shape=mixed]) -> obj<8x4> [result_shape=mixed]
declare function @empty(%x : obj<1x1> [shape=empty]) -> obj<1x1> [result_shape=empty]
