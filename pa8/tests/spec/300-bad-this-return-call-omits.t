target wasm32 1
declare function @initialize(%self : ptr) -> void [this_return=yes]
function @run(%self : ptr) -> void {
  block ^entry:
    call void @initialize(%self) as (%arg0 : ptr) -> void
    return void
}
