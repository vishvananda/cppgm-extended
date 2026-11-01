# Typed accesses of incompatible types do not overlap. A load reads what the
# load before it read when only such stores come between.

function @float_over_int(%p : ptr, %q : ptr, %v : f32) -> i32 [no_inline=yes] {
  block ^entry:
    %a = load typed i32 %p
    store typed f32 %v, %q
    %b = load typed i32 %p
    %r = binary add i32 %a, %b
    return i32 %r
}

function @double_over_float(%p : ptr, %q : ptr, %v : f64) -> f32 [no_inline=yes] {
  block ^entry:
    %a = load typed f32 %p
    store typed f64 %v, %q
    %b = load typed f32 %p
    %r = binary add f32 %a, %b
    return f32 %r
}

function @long_over_int(%p : ptr, %q : ptr, %v : i64) -> i32 [no_inline=yes] {
  block ^entry:
    %a = load typed i32 %p
    store typed i64 %v, %q
    %b = load typed i32 %p
    %r = binary add i32 %a, %b
    return i32 %r
}

function @short_over_int(%p : ptr, %q : ptr, %v : i16) -> i32 [no_inline=yes] {
  block ^entry:
    %a = load typed i32 %p
    store typed i16 %v, %q
    %b = load typed i32 %p
    %r = binary add i32 %a, %b
    return i32 %r
}

function @int_over_pointer(%p : ptr, %q : ptr, %v : i32) -> ptr [no_inline=yes] {
  block ^entry:
    %a = load typed ptr %p
    store typed i32 %v, %q
    %b = load typed ptr %p
    %c = cmp eq ptr %a, %b
    %r = select ptr %c, %a, %p
    return ptr %r
}

# Compatible: an integer of one width whatever its sign, one floating type,
# two pointers.
function @signs_agree(%p : ptr, %q : ptr, %v : u32) -> i32 [no_inline=yes] {
  block ^entry:
    %a = load typed i32 %p
    store typed u32 %v, %q
    %b = load typed i32 %p
    %r = binary add i32 %a, %b
    return i32 %r
}

function @floats_agree(%p : ptr, %q : ptr, %v : f64) -> f64 [no_inline=yes] {
  block ^entry:
    %a = load typed f64 %p
    store typed f64 %v, %q
    %b = load typed f64 %p
    %r = binary add f64 %a, %b
    return f64 %r
}

function @pointers_agree(%p : ptr, %q : ptr, %v : ptr) -> ptr [no_inline=yes] {
  block ^entry:
    %a = load typed ptr %p
    store typed ptr %v, %q
    %b = load typed ptr %p
    %c = cmp eq ptr %a, %b
    %r = select ptr %c, %a, %p
    return ptr %r
}

# Not stated: either access may be any object's.
function @store_not_typed(%p : ptr, %q : ptr, %v : f32) -> i32 [no_inline=yes] {
  block ^entry:
    %a = load typed i32 %p
    store f32 %v, %q
    %b = load typed i32 %p
    %r = binary add i32 %a, %b
    return i32 %r
}

function @load_not_typed(%p : ptr, %q : ptr, %v : f32) -> i32 [no_inline=yes] {
  block ^entry:
    %a = load i32 %p
    store typed f32 %v, %q
    %b = load i32 %p
    %r = binary add i32 %a, %b
    return i32 %r
}

# A byte access may be a character type's.
function @byte_store(%p : ptr, %q : ptr, %v : i8) -> i32 [no_inline=yes] {
  block ^entry:
    %a = load typed i32 %p
    store i8 %v, %q
    %b = load typed i32 %p
    %r = binary add i32 %a, %b
    return i32 %r
}

# The load of the extent leaves a loop that stores doubles through a pointer.
function @extent_leaves_loop(%p : ptr, %q : ptr, %v : f64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %n = load typed i64 %p
    %at = index i64 %q, %i
    store typed f64 %v, %at
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# A store of the same width stays in the way.
function @extent_stays(%p : ptr, %q : ptr, %v : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^head: %next]
    %n = load typed i64 %p
    %at = index i64 %q, %i
    store typed i64 %v, %at
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# A store on one path through the loop: the load in the header moves past it.
function @branchy_extent(%p : ptr, %q : ptr, %v : f64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %n = load typed i64 %p
    %y = binary and i64 %i, 1
    %z = cmp eq i64 %y, 0
    branch %z, ^write, ^tail
  block ^write:
    %at = index i64 %q, %i
    store typed f64 %v, %at
    jump ^tail
  block ^tail:
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}

# Not stated: the store may write the extent.
function @branchy_untyped(%p : ptr, %q : ptr, %v : f64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^tail: %next]
    %n = load typed i64 %p
    %y = binary and i64 %i, 1
    %z = cmp eq i64 %y, 0
    branch %z, ^write, ^tail
  block ^write:
    %at = index i64 %q, %i
    store f64 %v, %at
    jump ^tail
  block ^tail:
    %next = binary add i64 %i, 1
    %c = cmp ult i64 %next, %n
    branch %c, ^head, ^done
  block ^done:
    return void
}
