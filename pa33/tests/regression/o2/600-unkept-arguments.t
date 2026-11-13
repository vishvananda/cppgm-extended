declare function @external(%p : ptr) -> void [linkage=c, binding=strong, object=external, unwind=no]

global @saved : ptr = 0

# Writes through its parameter and through the cursor it holds; keeps neither.
function @append(%s : ptr, %c : i8) -> void [no_inline=yes] {
  block ^entry:
    %cur = load ptr %s
    %next = index i8 %cur, 1
    store ptr %next, %s
    store i8 %c, %cur
    return void
}

# Passes its parameter only to a callee that does not keep it.
function @append_twice(%s : ptr, %c : i8) -> void [no_inline=yes] {
  block ^entry:
    call void @append(%s, %c)
    call void @append(%s, %c)
    return void
}

# Keeps its parameter: a store of the address.
function @keep(%s : ptr) -> void [no_inline=yes] {
  block ^entry:
    store ptr %s, @saved
    return void
}

# Keeps it: a callee this unit does not define is given the address.
function @pass_on(%s : ptr) -> void [no_inline=yes] {
  block ^entry:
    call void @external(%s)
    return void
}

# Keeps it: the address is returned.
function @same(%s : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    return ptr %s
}

# The cursor's slot is given only to @append. The call writes it: the load
# after the call stays. The byte store through the cursor cannot reach the
# slot: the reload after it folds to the stored cursor.
function @cursor(%start : ptr, %c : i8) -> ptr [no_inline=yes] {
  slot $s : ptr
  block ^entry:
    %a = addr $s
    store ptr %start, %a
    call void @append(%a, %c)
    %p = load ptr %a
    %n = index i8 %p, 1
    store ptr %n, %a
    store i8 %c, %p
    %r = load ptr %a
    return ptr %r
}

# The same through a callee that passes the address on.
function @cursor_twice(%start : ptr, %c : i8) -> ptr [no_inline=yes] {
  slot $s : ptr
  block ^entry:
    %a = addr $s
    store ptr %start, %a
    call void @append_twice(%a, %c)
    %p = load ptr %a
    %n = index i8 %p, 1
    store ptr %n, %a
    store i8 %c, %p
    %r = load ptr %a
    return ptr %r
}

# A kept address may be reached through the cursor: each reload stays.
function @kept_by_store(%start : ptr, %c : i8) -> ptr [no_inline=yes] {
  slot $s : ptr
  block ^entry:
    %a = addr $s
    store ptr %start, %a
    call void @keep(%a)
    %p = load ptr %a
    %n = index i8 %p, 1
    store ptr %n, %a
    store i8 %c, %p
    %r = load ptr %a
    return ptr %r
}

function @kept_by_external(%start : ptr, %c : i8) -> ptr [no_inline=yes] {
  slot $s : ptr
  block ^entry:
    %a = addr $s
    store ptr %start, %a
    call void @pass_on(%a)
    %p = load ptr %a
    %n = index i8 %p, 1
    store ptr %n, %a
    store i8 %c, %p
    %r = load ptr %a
    return ptr %r
}

function @kept_by_return(%start : ptr, %c : i8) -> ptr [no_inline=yes] {
  slot $s : ptr
  block ^entry:
    %a = addr $s
    store ptr %start, %a
    %b = call ptr @same(%a)
    %p = load ptr %b
    %n = index i8 %p, 1
    store ptr %n, %b
    store i8 %c, %p
    %r = load ptr %a
    return ptr %r
}

# The callee reads what is stored before the call: no load here reads the
# slot, but the store stays.
function @read_by_call(%start : ptr, %c : i8) -> void [no_inline=yes] {
  slot $s : ptr
  block ^entry:
    %a = addr $s
    store ptr %start, %a
    call void @append(%a, %c)
    return void
}

# The slot holds its own address: the callee may load that address and keep
# a copy. The store of the address stays.
function @own_address(%c : i8) -> void [no_inline=yes] {
  slot $s : ptr
  block ^entry:
    %a = addr $s
    store ptr %a, %a
    call void @append(%a, %c)
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
