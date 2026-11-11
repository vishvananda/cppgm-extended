target wasm32 1
declare function @sum(%n : i32) -> i64 [arity=variadic]
function @forward(%integer : i32, %floating : f64, %address : ptr) -> i64 {
  block ^entry:
    %result = call i64 @sum(3, %integer, %floating, %address) [argument_type=1:i32, argument_type=2:f64, argument_type=3:ptr]
    return i64 %result
}
