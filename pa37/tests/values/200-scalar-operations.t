target wasm32 1

function @arithmetic() -> i32 {
  block ^entry:
    %x = binary mul i32 6, 7
    %y = binary sub i32 %x, 1
    return i32 %y
}

function @signed_narrow() -> i32 {
  block ^entry:
    %x = const i8 -3
    %y = convert sext i32 i8 %x
    return i32 %y
}

function @wide() -> i64 {
  block ^entry:
    %x = binary mul i64 1234567, 1000000
    %y = binary add i64 %x, 89
    return i64 %y
}

function @floating() -> f64 {
  block ^entry:
    %x = binary mul f64 1.5, 4.0
    %y = binary sub f64 %x, 0.5
    return f64 %y
}
