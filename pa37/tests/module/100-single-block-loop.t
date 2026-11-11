target wasm32 1
function @repeat_forever() -> void {
  block ^loop:
    jump ^loop
}
