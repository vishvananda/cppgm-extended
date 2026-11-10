# A store to a slot goes when a later store in its block replaces all of it
# and nothing between them may read it. Each function below passes the slot
# to @consume, so the slot keeps its storage.
global @seen : obj<8x8> = { zero 8 }

function @keep(%p : ptr) -> void [no_inline=yes] {
  block ^entry:
    store ptr %p, @seen
    return void
}

function @consume(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    %first = load i64 %p
    %at = index i8 %p, 8
    %second = load i64 %at
    %sum = binary add i64 %first, %second
    return i64 %sum
}

function @replaced(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %q = index i8 %p, 8
    store i64 %a, %p
    store i64 %a, %q
    store i64 %b, %p
    store i64 %b, %q
    %r = call i64 @consume(%p)
    return i64 %r
}

function @wider_replaces_two(%a : i32, %b : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %q = index i8 %p, 4
    %h = index i8 %p, 8
    store i32 %a, %p
    store i32 %a, %q
    store i64 %b, %p
    store i64 0, %h
    %r = call i64 @consume(%p)
    return i64 %r
}

function @narrower_keeps(%a : i64, %b : i32) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %h = index i8 %p, 8
    store i64 %a, %p
    store i32 %b, %p
    store i64 0, %h
    %r = call i64 @consume(%p)
    return i64 %r
}

function @read_through_a_pointer_keeps(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %h = index i8 %p, 8
    store i64 0, %h
    call void @keep(%p)
    %known = load ptr @seen
    store i64 %a, %p
    %read = load i64 %known
    store i64 %b, %p
    %r = call i64 @consume(%p)
    %sum = binary add i64 %r, %read
    return i64 %sum
}

function @read_elsewhere_in_the_slot_goes(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %h = index i8 %p, 8
    store i64 7, %h
    call void @keep(%p)
    store i64 %a, %p
    %read = load i64 %h
    store i64 %b, %p
    %r = call i64 @consume(%p)
    %sum = binary add i64 %r, %read
    return i64 %sum
}

function @call_between_keeps(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %h = index i8 %p, 8
    store i64 0, %h
    store i64 %a, %p
    %read = call i64 @consume(%p)
    store i64 %b, %p
    %r = call i64 @consume(%p)
    %sum = binary add i64 %r, %read
    return i64 %sum
}

function @volatile_keeps(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %h = index i8 %p, 8
    store i64 0, %h
    store volatile i64 %a, %p
    store volatile i64 %b, %p
    %r = call i64 @consume(%p)
    return i64 %r
}

function @f80_covers_ten_bytes(%a : i64, %x : f80) -> i64 [no_inline=yes] {
  slot $s : obj<16x16>
  block ^entry:
    %p = addr $s
    %h = index i8 %p, 8
    store i64 %a, %h
    store f80 %x, %p
    %r = call i64 @consume(%p)
    return i64 %r
}

function @other_slot_keeps(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  slot $t : obj<16x8>
  block ^entry:
    %p = addr $s
    %h = index i8 %p, 8
    %q = addr $t
    %k = index i8 %q, 8
    store i64 0, %h
    store i64 0, %k
    store i64 %a, %p
    store i64 %b, %q
    %r = call i64 @consume(%p)
    %o = call i64 @consume(%q)
    %sum = binary add i64 %r, %o
    return i64 %sum
}

function @volatile_load_keeps(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %h = index i8 %p, 8
    store i64 0, %h
    store i64 %a, %p
    %v = load volatile ptr @seen
    store i64 %b, %p
    %r = call i64 @consume(%p)
    return i64 %r
}

function @other_path_reads(%a : i64, %b : i64, %c : i32) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %h = index i8 %p, 8
    store i64 0, %h
    store i64 %a, %p
    branch %c, ^next, ^other
  block ^next:
    store i64 %b, %p
    %r = call i64 @consume(%p)
    return i64 %r
  block ^other:
    %o = call i64 @consume(%p)
    return i64 %o
}

function @fence_keeps(%a : i64, %b : i64) -> i64 [no_inline=yes] {
  slot $s : obj<16x8>
  block ^entry:
    %p = addr $s
    %h = index i8 %p, 8
    store i64 0, %h
    call void @keep(%p)
    store i64 %a, %p
    atomic_thread_fence 5
    store i64 %b, %p
    %r = call i64 @consume(%p)
    return i64 %r
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %r1 = call i64 @replaced(1, 2)
    %r2 = call i64 @wider_replaces_two(1, 3)
    %r3 = call i64 @narrower_keeps(4294967296, 5)
    %r4 = call i64 @read_through_a_pointer_keeps(6, 7)
    %r5 = call i64 @read_elsewhere_in_the_slot_goes(1, 2)
    %r6 = call i64 @call_between_keeps(1, 2)
    %r7 = call i64 @volatile_keeps(1, 2)
    %r8 = call i64 @other_path_reads(1, 2, 0)
    %r9 = call i64 @fence_keeps(1, 2)
    %r10 = call i64 @other_slot_keeps(1, 2)
    %r11 = call i64 @volatile_load_keeps(1, 2)
    %s1 = binary add i64 %r1, %r2
    %s2 = binary add i64 %s1, %r3
    %s3 = binary add i64 %s2, %r4
    %s4 = binary add i64 %s3, %r5
    %s5 = binary add i64 %s4, %r6
    %s6 = binary add i64 %s5, %r7
    %s7 = binary add i64 %s6, %r8
    %s8 = binary add i64 %s7, %r9
    %s9 = binary add i64 %s8, %r10
    %s10 = binary add i64 %s9, %r11
    %out = convert trunc i32 i64 %s10
    return i32 %out
}
