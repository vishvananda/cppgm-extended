# Zero subranges can share memory operations without touching holes or
# nonzero fields. Repeated ordinary writes do not make a range observable.
global @fields : obj<64x8> = { i64 11 i64 22 i64 77 i64 33 i64 44 i64 55 i64 66 i64 88 }

function @sparse_fields(%p : ptr) -> void [unwind=no] {
  block ^entry:
    %p1 = index i64 %p, 1
    %p3 = index i64 %p, 3
    %p4 = index i64 %p, 4
    %p5 = index i64 %p, 5
    %p6 = index i64 %p, 6
    %p7 = index i64 %p, 7
    store i64 0, %p
    store i64 0, %p1
    store i64 0, %p3
    store i64 0, %p4
    store i64 0, %p4
    store i64 0, %p5
    store i64 0, %p6
    store i64 9, %p7
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    call void @sparse_fields(@fields)
    jump ^loop
  block ^loop:
    %i = phi i64 [^entry: 0, ^next: %inext]
    %p = index i64 @fields, %i
    %value = load i64 %p
    %is_hole = cmp eq i64 %i, 2
    %is_nonzero = cmp eq i64 %i, 7
    %nonzero = select i64 %is_nonzero, 9, 0
    %expected = select i64 %is_hole, 77, %nonzero
    %bad = cmp ne i64 %value, %expected
    branch %bad, ^fail, ^next
  block ^next:
    %inext = binary add i64 %i, 1
    %more = cmp ult i64 %inext, 8
    branch %more, ^loop, ^done
  block ^fail:
    return i32 1
  block ^done:
    return i32 0
}
