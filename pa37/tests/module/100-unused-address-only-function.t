target wasm32 1
declare function @unused
declare function @other_unused [object=external_name]
function @observe() -> i32 {
  block ^entry:
    return i32 17
}
