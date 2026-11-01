# What the wide loop must keep of the order of a loop's accesses.

# The load reads ahead of the store.
function @reads_ahead(%a : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = binary add nowrap i64 %i, 1
    %p = index i32 %a, %j
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %a, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# The same element.
function @same_element(%a : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = binary add nowrap i64 %i, 0
    %p = index i32 %a, %j
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %a, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: the load reads what an earlier pass stored.
function @reads_behind(%a : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = binary add nowrap i64 %i, -1
    %p = index i32 %a, %j
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %a, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @reads_behind_3(%a : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = binary add nowrap i64 %i, -3
    %p = index i32 %a, %j
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %a, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# A whole vector apart.
function @apart_behind(%a : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = binary add nowrap i64 %i, -4
    %p = index i32 %a, %j
    %v = load typed i32 %p
    %r = binary add i32 %v, 1
    %o = index i32 %a, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: the store comes first and the later load would see the next lanes.
function @store_first(%a : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %o = index i32 %a, %i
    store typed i32 1, %o
    %j = binary add nowrap i64 %i, 1
    %p = index i32 %a, %j
    %v = load typed i32 %p
    %q = index i32 %a, %i
    %w = binary add i32 %v, 1
    %z = index i32 %a, %j
    store typed i32 %w, %z
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Two arrays of one object have their ranges tested.
global @table : obj<4000x4> = { zero 4000 }
global @other : obj<4000x4> = { zero 4000 }

function @rows_of_one_object(%k : i64, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = binary add nowrap i64 %i, %k
    %p = index i32 @table, %j
    %v = load typed i32 %p
    %o = index i32 @table, %i
    store typed i32 %v, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Named objects need no test.
function @two_objects(%n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 @other, %i
    %v = load typed i32 %p
    %o = index i32 @table, %i
    store typed i32 %v, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Types apart need no test.
function @types_apart(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load typed f32 %p
    %r = convert fptosi i32 f32 %v
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Untyped accesses need the test.
function @untyped(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load i32 %p
    %o = index i32 %d, %i
    store i32 %v, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# One test for each pair of arrays: at most eight.
function @three_stores(%a0 : ptr, %a1 : ptr, %a2 : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load i32 %p
    %o0 = index i32 %a0, %i
    store i32 %v, %o0
    %o1 = index i32 %a1, %i
    store i32 %v, %o1
    %o2 = index i32 %a2, %i
    store i32 %v, %o2
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: five arrays pair up in ten tests.
function @four_stores(%a0 : ptr, %a1 : ptr, %a2 : ptr, %a3 : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load i32 %p
    %o0 = index i32 %a0, %i
    store i32 %v, %o0
    %o1 = index i32 %a1, %i
    store i32 %v, %o1
    %o2 = index i32 %a2, %i
    store i32 %v, %o2
    %o3 = index i32 %a3, %i
    store i32 %v, %o3
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Refused: two stores of one array closer than the vector.
function @close_stores(%a : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %j = binary add nowrap i64 %i, 1
    %p = index i32 %a, %j
    store typed i32 1, %p
    %q = index i32 %a, %i
    store typed i32 2, %q
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# Types apart need both accesses typed.
function @one_typed(%d : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index f32 %x, %i
    %v = load f32 %p
    %r = convert fptosi i32 f32 %v
    %o = index i32 %d, %i
    store typed i32 %r, %o
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
