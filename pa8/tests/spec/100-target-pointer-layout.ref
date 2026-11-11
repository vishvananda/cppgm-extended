target wasm32 1

global @pointer : ptr = zero

function @step(%base : ptr) -> ptr {
  block ^entry:
    %next = index ptr %base, 1
    return ptr %next
}
