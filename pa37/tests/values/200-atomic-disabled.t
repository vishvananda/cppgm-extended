target wasm32 1
function @observe(%address : ptr) -> i32 {
  block ^entry:
    %value = atomic_load i32 %address, 2
    return i32 %value
}
