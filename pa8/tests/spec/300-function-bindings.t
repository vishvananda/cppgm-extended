target wasm32 1
import @external "host @name" "field\"name"
declare function @external(%value : i32) -> i32 [object=linking_name]
declare function @empty(%value : i64) -> i64
import @empty "" ""
function @wrapper(%value : i32) -> i32 [binding=internal] {
  block ^entry:
    %result = call i32 @external(%value)
    return i32 %result
}
export @wrapper "public café"
export @external "forwarded"
