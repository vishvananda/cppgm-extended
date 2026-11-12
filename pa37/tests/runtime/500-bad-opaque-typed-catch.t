target wasm32 1
global @type_info : ptr = zero
function @bad() -> i32 {
 block ^entry:
  eh_try ^handler
  throw i32 17
 block ^handler:
  eh_catch @type_info, 1
  return i32 0
}
