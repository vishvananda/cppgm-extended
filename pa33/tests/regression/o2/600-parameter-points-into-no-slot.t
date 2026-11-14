# A parameter held its value before the function's frame existed, so it
# points into no slot of the function, even a slot whose address a call
# took. A store into such a slot keeps what was read through the parameter:
# the second read reuses the first, and a loop that stores into the slot reads
# the parameter's object once, before it. A pointer loaded from memory may
# point into the slot the call took: its reads stay.
declare function @observe(%address : ptr) -> void [linkage=c, binding=strong]

function @read_twice(%p : ptr) -> i64 [binding=strong, no_inline=yes] {
  slot $buffer : obj<16x8>

  block ^entry:
    %b = addr $buffer
    store i64 1, %b
    call void @observe(%b)
    %first = load i64 %p
    store i64 %first, %b
    %second = load i64 %p
    %sum = binary add i64 %first, %second
    call void @observe(%b)
    return i64 %sum
}

function @loaded_pointer(%holder : ptr) -> i64 [binding=strong, no_inline=yes] {
  slot $buffer : obj<16x8>

  block ^entry:
    %b = addr $buffer
    store i64 1, %b
    call void @observe(%b)
    %q = load ptr %holder
    %first = load i64 %q
    store i64 %first, %b
    %second = load i64 %q
    %sum = binary add i64 %first, %second
    call void @observe(%b)
    return i64 %sum
}

function @in_loop(%p : ptr, %n : i64) -> i64 [binding=strong, no_inline=yes] {
  slot $buffer : obj<16x8>

  block ^entry:
    %b = addr $buffer
    store i64 0, %b
    call void @observe(%b)
    jump ^loop

  block ^loop:
    %i = phi i64 [^entry: 0, ^loop: %next]
    %step = load i64 %p
    %old = load i64 %b
    %new = binary add i64 %old, %step
    store i64 %new, %b
    %next = binary add nowrap i64 %i, 1
    %again = cmp lt i64 %next, %n
    branch %again, ^loop, ^done

  block ^done:
    call void @observe(%b)
    %r = load i64 %b
    return i64 %r
}
