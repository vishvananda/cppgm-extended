# Consecutive ordinary zero fields may use fewer memory operations; holes,
# nonzero fields and separately observable accesses retain their effects.
global @zeroed : obj<80x8> = { i64 -1 i64 -1 i64 -1 i64 -1 i64 -1 i64 -1 i64 -1 i64 -1 i64 -1 i64 -1 }
global @holes : obj<24x8> = { i64 11 i64 22 i64 33 }
global @mixed : obj<24x8> = { i64 11 i64 22 i64 33 }
global @volatiles : obj<16x8> = { i64 11 i64 22 }
global @atomics : obj<16x8> = { i64 11 i64 22 }

function @zero_fields(%p : ptr) -> void [unwind=no] {
  block ^entry:
    %p1 = index i64 %p, 1
    %p2 = index i64 %p, 2
    %p3 = index i64 %p, 3
    %p4 = index i64 %p, 4
    %p5 = index i64 %p, 5
    %p6 = index i64 %p, 6
    %p7 = index i64 %p, 7
    %p8 = index i64 %p, 8
    %p9 = index i64 %p, 9
    store i64 0, %p
    store i64 0, %p1
    store i64 0, %p2
    store i64 0, %p3
    store i64 0, %p4
    store i64 0, %p5
    store i64 0, %p6
    store i64 0, %p7
    store i64 0, %p8
    store i64 0, %p9
    return void
}

function @leave_hole(%p : ptr) -> void [unwind=no] {
  block ^entry:
    %p2 = index i64 %p, 2
    store i64 0, %p
    store i64 0, %p2
    return void
}

function @keep_nonzero(%p : ptr) -> void [unwind=no] {
  block ^entry:
    %p1 = index i64 %p, 1
    %p2 = index i64 %p, 2
    store i64 0, %p
    store i64 9, %p1
    store i64 0, %p2
    return void
}

function @volatile_fields(%p : ptr) -> void [unwind=no] {
  block ^entry:
    %p1 = index i64 %p, 1
    store volatile i64 0, %p
    store volatile i64 0, %p1
    return void
}

function @atomic_fields(%p : ptr) -> void [unwind=no] {
  block ^entry:
    %p1 = index i64 %p, 1
    atomic_store i64 0, %p, 0
    atomic_store i64 0, %p1, 0
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    call void @zero_fields(@zeroed)
    call void @leave_hole(@holes)
    call void @keep_nonzero(@mixed)
    call void @volatile_fields(@volatiles)
    call void @atomic_fields(@atomics)
    %z9 = index i64 @zeroed, 9
    %last = load i64 %z9
    %first = load i64 @zeroed
    %h1 = index i64 @holes, 1
    %hole = load i64 %h1
    %m1 = index i64 @mixed, 1
    %nonzero = load i64 %m1
    %v1 = index i64 @volatiles, 1
    %v = load volatile i64 %v1
    %a1 = index i64 @atomics, 1
    %a = atomic_load i64 %a1, 0
    %bad_first = cmp ne i64 %first, 0
    %bad_last = cmp ne i64 %last, 0
    %bad_hole = cmp ne i64 %hole, 22
    %bad_nonzero = cmp ne i64 %nonzero, 9
    %bad_volatile = cmp ne i64 %v, 0
    %bad_atomic = cmp ne i64 %a, 0
    %bad0 = binary or i32 %bad_first, %bad_last
    %bad1 = binary or i32 %bad_hole, %bad_nonzero
    %bad2 = binary or i32 %bad_volatile, %bad_atomic
    %bad3 = binary or i32 %bad0, %bad1
    %bad = binary or i32 %bad3, %bad2
    return i32 %bad
}
