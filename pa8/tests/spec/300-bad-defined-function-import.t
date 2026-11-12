target wasm32 1
function @defined() -> i32 {
  block ^entry:
    return i32 17
}
import @defined "host" "field"
