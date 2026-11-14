target wasm32 1
function @observe(%address : ptr) -> i128 {
  block ^entry:
    %value = atomic_load i128 %address, 2
    return i128 %value
}
