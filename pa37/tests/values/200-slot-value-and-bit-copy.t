target wasm32 1

function @slot_value() -> i32 {
  slot $value : i32
  block ^entry:
    store i32 40, $value
    %sum = binary add i32 $value, 2
    return i32 %sum
}

function @float_bits() -> i32 {
  block ^entry:
    %bits = const i32 1065353216
    %value = copy f32 %bits
    %result = convert fptosi i32 f32 %value
    return i32 %result
}
