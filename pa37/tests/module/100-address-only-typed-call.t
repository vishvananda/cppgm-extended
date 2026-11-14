target wasm32 1
declare function @external
function @observe() -> i32 {
  block ^entry:
    %answer = call i32 @external(20) as (%value : i32) -> i32
    return i32 %answer
}
