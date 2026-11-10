# Consume loaded integer operands before loading the rest of a long sum.
# The call in the second function keeps its operands live across it, and
# changes the first element after it was read. Memory accesses keep order.

function @touch(%p : ptr) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 %p
    %new = binary add i64 %old, 100
    store i64 %new, %p
    return void
}

function @sum(%p : ptr, %a : i64, %b : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %p0 = index i64 %p, 0
    %x0 = load typed i64 %p0
    %p1 = index i64 %p, 1
    %x1 = load typed i64 %p1
    %p2 = index i64 %p, 2
    %x2 = load typed i64 %p2
    %p3 = index i64 %p, 3
    %x3 = load typed i64 %p3
    %p4 = index i64 %p, 4
    %x4 = load typed i64 %p4
    %p5 = index i64 %p, 5
    %x5 = load typed i64 %p5
    %p6 = index i64 %p, 6
    %x6 = load typed i64 %p6
    %p7 = index i64 %p, 7
    %x7 = load typed i64 %p7
    %m0 = binary mul i64 %x0, %a
    %m1 = binary mul i64 %x1, %b
    %m2 = binary mul i64 %x2, %a
    %m3 = binary mul i64 %x3, %b
    %m4 = binary mul i64 %x4, %a
    %m5 = binary mul i64 %x5, %b
    %m6 = binary mul i64 %x6, %a
    %m7 = binary mul i64 %x7, %b
    %s1 = binary add i64 %m0, %m1
    %s2 = binary add i64 %s1, %m2
    %s3 = binary add i64 %s2, %m3
    %s4 = binary add i64 %s3, %m4
    %s5 = binary add i64 %s4, %m5
    %s6 = binary add i64 %s5, %m6
    %s7 = binary add i64 %s6, %m7
    return i64 %s7
}

function @across_call(%p : ptr, %a : i64, %b : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %p0 = index i64 %p, 0
    %x0 = load typed i64 %p0
    %p1 = index i64 %p, 1
    %x1 = load typed i64 %p1
    %p2 = index i64 %p, 2
    %x2 = load typed i64 %p2
    %p3 = index i64 %p, 3
    %x3 = load typed i64 %p3
    %p4 = index i64 %p, 4
    %x4 = load typed i64 %p4
    %p5 = index i64 %p, 5
    %x5 = load typed i64 %p5
    %p6 = index i64 %p, 6
    %x6 = load typed i64 %p6
    %p7 = index i64 %p, 7
    %x7 = load typed i64 %p7
    call void @touch(%p)
    %m0 = binary mul i64 %x0, %a
    %m1 = binary mul i64 %x1, %b
    %m2 = binary mul i64 %x2, %a
    %m3 = binary mul i64 %x3, %b
    %m4 = binary mul i64 %x4, %a
    %m5 = binary mul i64 %x5, %b
    %m6 = binary mul i64 %x6, %a
    %m7 = binary mul i64 %x7, %b
    %s1 = binary add i64 %m0, %m1
    %s2 = binary add i64 %s1, %m2
    %s3 = binary add i64 %s2, %m3
    %s4 = binary add i64 %s3, %m4
    %s5 = binary add i64 %s4, %m5
    %s6 = binary add i64 %s5, %m6
    %s7 = binary add i64 %s6, %m7
    return i64 %s7
}

function @main() -> i32 [role=entry] {
  slot $data : obj<64x8>
  block ^entry:
    %p = addr $data
    %p0 = index i64 %p, 0
    store i64 1, %p0
    %p1 = index i64 %p, 1
    store i64 2, %p1
    %p2 = index i64 %p, 2
    store i64 3, %p2
    %p3 = index i64 %p, 3
    store i64 4, %p3
    %p4 = index i64 %p, 4
    store i64 5, %p4
    %p5 = index i64 %p, 5
    store i64 6, %p5
    %p6 = index i64 %p, 6
    store i64 7, %p6
    %p7 = index i64 %p, 7
    store i64 8, %p7
    %a = call i64 @sum(%p, 3, 5)
    %b = call i64 @across_call(%p, 3, 5)
    %first = load i64 %p
    %ok_a = cmp eq i64 %a, 148
    %ok_b = cmp eq i64 %b, 148
    %ok_first = cmp eq i64 %first, 101
    %ok_both = binary and i64 %ok_a, %ok_b
    %ok = binary and i64 %ok_both, %ok_first
    %rc = select i32 %ok, 0, 1
    return i32 %rc
}
