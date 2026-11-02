# A sum kept at an invariant address is a reduction. The lanes load it first and
# store it last, after a test that the array it adds is apart from it. A 64-bit
# counter that copies a 32-bit one steps with it.

function @row(%out : ptr, %x : ptr, %y : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i32 [^entry: 0, ^body: %next]
    %w = phi i64 [^entry: 0, ^body: %wide]
    %p = index f64 %x, %w
    %v = load typed f64 %p
    %q = index f64 %y, %w
    %d = load typed f64 %q
    %m = binary div f64 %v, %d
    %old = load typed f64 %out
    %sum = binary add f64 %old, %m
    store typed f64 %sum, %out
    %next = binary add nowrap i32 %i, 1
    %wide = convert sext i64 i32 %next
    %c = cmp ult i64 %wide, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# An integer sum in a field: its address stays beside the load and the store.
function @field(%this : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %cell = index i8 %this, 16
    %old = load typed i32 %cell
    %sum = binary add i32 %old, %v
    store typed i32 %sum, %cell
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# The types keep this sum apart from the array: no test.
function @typed(%out : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %wide = convert sext i64 i32 %v
    %old = load typed i64 %out
    %sum = binary add i64 %old, %wide
    store typed i64 %sum, %out
    %next = binary add i64 %i, 1
    %c = cmp lt i64 %next, %n
    branch %c, ^body, ^done
  block ^done:
    return void
}

# The store writes another value than the sum: the loop stays scalar.
function @refused(%out : ptr, %x : ptr, %n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %p = index i32 %x, %i
    %v = load typed i32 %p
    %old = load typed i32 %out
    %sum = binary add i32 %old, %v
    %twice = binary add i32 %sum, %sum
    store typed i32 %twice, %out
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
