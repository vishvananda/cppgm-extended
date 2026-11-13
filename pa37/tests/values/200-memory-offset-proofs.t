# Memory immediates must not acquire the wrapping semantics of a plain index.
target wasm32 1

global @buffer : obj<64x4> = zero

function @folded_load(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %q = index inbounds i32 %p, 3
    %value = load i32 %q
    return i32 %value
}
function @folded_store(%p : ptr, %value : i32) -> void [no_inline=yes] {
  block ^entry:
    %q = index inbounds i32 %p, 3
    store volatile i32 %value, %q
    return void
}
function @copied_load(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %first = index inbounds i8 %p, 4
    %copy = copy ptr %first
    %q = index inbounds i32 %copy, 2
    %value = load i32 %q
    return i32 %value
}
function @negative_load(%p : ptr) -> i32 [no_inline=yes] {
  block ^entry:
    %q = index inbounds i8 %p, -4
    %value = load i32 %q
    return i32 %value
}
function @address_used(%p : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    %q = index inbounds i32 %p, 3
    store volatile i32 73, %q
    return ptr %q
}
function @slot_copy(%p : ptr) -> i32 [no_inline=yes] {
  slot $saved : ptr
  block ^entry:
    store ptr %p, $saved
    %copy = copy ptr $saved
    %q = index inbounds i32 %copy, 3
    %value = load i32 %q
    return i32 %value
}
function @exercise() -> i32 {
  block ^entry:
    %base = addr @buffer
    call void @folded_store(%base, 42)
    %first = call i32 @folded_load(%base)
    %second = call i32 @copied_load(%base)
    %end = index inbounds i8 %base, 16
    %third = call i32 @negative_load(%end)
    %fifth = call i32 @slot_copy(%base)
    %used = call ptr @address_used(%base)
    %fourth = load volatile i32 %used
    %pair = binary add i32 %first, %second
    %triple = binary add i32 %pair, %third
    %sum = binary add i32 %triple, %fourth
    %total = binary add i32 %sum, %fifth
    return i32 %total
}
function @plain_field(%p : ptr) -> i32 {
  block ^entry:
    %low = const ptr 16
    store volatile i32 91, %low
    %q = index i8 [projection=field] %p, 32
    %value = load volatile i32 %q
    return i32 %value
}
function @scaled_wrap(%p : ptr) -> i32 {
  block ^entry:
    %low = const ptr 16
    store volatile i32 92, %low
    %q = index i32 %p, 8
    %value = load volatile i32 %q
    return i32 %value
}
function @negative_wrap(%p : ptr) -> i32 {
  block ^entry:
    %low = const ptr 0
    store volatile i32 93, %low
    %q = index i8 %p, -8
    %value = load volatile i32 %q
    return i32 %value
}
function @plain_copy(%p : ptr) -> i32 {
  block ^entry:
    %low = const ptr 16
    store volatile i32 94, %low
    %q = index i8 [projection=field] %p, 32
    %copy = copy ptr %q
    %value = load volatile i32 %copy
    return i32 %value
}
