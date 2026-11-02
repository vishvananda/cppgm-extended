declare function @malloc(%n : i64) -> ptr [linkage=c, binding=strong, object=malloc, unwind=no]
declare function @touch(%p : ptr) -> void [linkage=c, binding=strong, object=touch, unwind=no]
declare function @__errno_location() -> ptr [linkage=c, binding=strong, object=__errno_location, unwind=no]

global @shared : ptr = 0
global @count : i64 = 0

# A parameter does not reach storage allocated here: the load folds.
function @past_a_parameter(%out : ptr, %v : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %p = call ptr @malloc(16)
    store i64 %v, %p
    store i64 7, %out
    %r = load i64 %p
    return i64 %r
}

# The stored address is reloaded past a store through it.
function @address_stays(%slot : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    %p = call ptr @malloc(16)
    store ptr %p, %slot
    store ptr 0, %p
    %q = load ptr %slot
    return ptr %q
}

# Nor does a global, and two allocations are apart.
function @past_others(%v : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %p = call ptr @malloc(16)
    %q = call ptr @malloc(16)
    store i64 %v, %p
    store i64 1, %q
    store i64 2, @count
    %r = load i64 %p
    return i64 %r
}

# What never leaves stays through a call, and through a load's pointer.
function @kept_private(%from : ptr, %v : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %p = call ptr @malloc(16)
    store i64 %v, %p
    call void @touch(%from)
    %other = load ptr %from
    store i64 3, %other
    %r = load i64 %p
    return i64 %r
}

# An allocation call keeps what it cannot change.
function @through_the_call(%this : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %a = load i64 %this
    %p = call ptr @malloc(16)
    store i64 1, %p
    %b = load i64 %this
    %r = binary add i64 %a, %b
    return i64 %r
}

# It left: another pointer may reach it, and the load stays.
function @after_it_left(%v : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %p = call ptr @malloc(16)
    store ptr %p, @shared
    store i64 %v, %p
    %other = load ptr @shared
    call void @touch(%other)
    %again = load ptr @shared
    store i64 9, %again
    %r = load i64 %p
    return i64 %r
}

# errno is an int behind a pointer: an allocation call may write it.
function @errno_cell() -> i32 [no_inline=yes] {
  block ^entry:
    %e = call ptr @__errno_location()
    store i32 0, %e
    %p = call ptr @malloc(16)
    %r = load i32 %e
    return i32 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
