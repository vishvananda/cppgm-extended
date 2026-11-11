target wasm32 1
function @one(%self : ptr) -> void [this_return=yes] {
 block ^entry:
  return void
}
function @two(%self : ptr) -> void {
 block ^entry:
  return void
}
equivalent_entry @one = @two
