target wasm32 1
declare function @external() -> i32
import @external "first" "field"
import @external "second" "field"
