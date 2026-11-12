target wasm32 1 [exception_abi=cxx]
function @bad() -> void {
 block ^entry:
  throw i32 17
}
