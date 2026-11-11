target wasm32 1
function @snapshot() -> i32 {
  slot $original : obj<8x4>
  slot $saved : obj<8x4>
  block ^entry:
    store i32 4, $original
    %value = load obj<8x4> $original
    store i32 19, $original
    store obj<8x4> %value, $saved
    %result = load i32 $saved
    return i32 %result
}
