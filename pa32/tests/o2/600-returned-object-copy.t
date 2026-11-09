global @published : ptr = 0

function @capture(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    store ptr %p, @published
    return void
}

# Returning the value does not publish its local storage's address.
function @returned_value(%source : ptr) -> obj<8x4> [no_inline=yes] {
  slot $out : obj<8x4>
  block ^entry:
    %destination = addr $out
    %a = load u32 %source
    %next = index u32 %source, 1
    %b = load u32 %next
    store u32 %a, %destination
    %high = index i8 %destination, 4
    store u32 %b, %high
    return obj<8x4> $out
}

# A captured address remains exposed even when the object is returned.
function @captured_value(%source : ptr) -> obj<8x4> [no_inline=yes] {
  slot $out : obj<8x4>
  block ^entry:
    %destination = addr $out
    call void @capture(%destination)
    %a = load u32 %source
    %next = index u32 %source, 1
    %b = load u32 %next
    store u32 %a, %destination
    %high = index i8 %destination, 4
    store u32 %b, %high
    return obj<8x4> $out
}

function @returned_address(%source : ptr) -> ptr [no_inline=yes] {
  slot $out : obj<8x4>
  block ^entry:
    %destination = addr $out
    %a = load u32 %source
    %next = index u32 %source, 1
    %b = load u32 %next
    store u32 %a, %destination
    %high = index i8 %destination, 4
    store u32 %b, %high
    return ptr %destination
}

function @volatile_value(%source : ptr) -> obj<8x4> [no_inline=yes] {
  slot $out : obj<8x4>
  block ^entry:
    %destination = addr $out
    %a = load volatile u32 %source
    %next = index u32 %source, 1
    %b = load volatile u32 %next
    store u32 %a, %destination
    %high = index i8 %destination, 4
    store u32 %b, %high
    return obj<8x4> $out
}

function @observed_frame(%source : ptr) -> obj<8x4> [no_inline=yes] {
  slot $out : obj<8x4>
  block ^entry:
    %frame = frame_address
    call void @capture(%frame)
    %destination = addr $out
    %a = load u32 %source
    %next = index u32 %source, 1
    %b = load u32 %next
    store u32 %a, %destination
    %high = index i8 %destination, 4
    store u32 %b, %high
    return obj<8x4> $out
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
