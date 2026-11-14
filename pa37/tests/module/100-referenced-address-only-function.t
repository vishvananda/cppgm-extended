target wasm32 1
declare function @opaque
function @observe() -> i32 {
  block ^entry:
    %nonzero = cmp ne ptr @opaque, nullptr
    branch %nonzero, ^yes, ^no
  block ^yes:
    return i32 17
  block ^no:
    return i32 0
}
