# A v128 value is a temporary or the literal 0.
function @f(%p : ptr) -> void {
  block ^entry:
    store v128 5, %p
    return void
}
