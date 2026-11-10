# A step may move after the final old-counter use, never after a use of its result.
function @walk(%p : ptr, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %sum = phi i64 [^entry: 0, ^latch: %total]
    %more = cmp ult i64 %i, %n
    branch %more, ^latch, ^exit
  block ^latch:
    %next = binary add i64 %i, 1
    %at = index i64 %p, %i
    %value = load i64 %at
    %part = binary add i64 %value, %i
    %total = binary add i64 %sum, %part
    jump ^header
  block ^exit:
    return i64 %sum
}
function @next_used(%n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %sum = phi i64 [^entry: 0, ^latch: %total]
    %more = cmp ult i64 %i, %n
    branch %more, ^latch, ^exit
  block ^latch:
    %next = binary add i64 %i, 1
    %first = binary add i64 %sum, %next
    %total = binary add i64 %first, %i
    jump ^header
  block ^exit:
    return i64 %sum
}
function @main() -> i64 [role=entry] {
  slot $data : obj<32x8>
  block ^entry:
    %p = addr $data
    %p1 = index i64 %p, 1
    %p2 = index i64 %p, 2
    %p3 = index i64 %p, 3
    store i64 7, %p
    store i64 11, %p1
    store i64 13, %p2
    store i64 17, %p3
    %a = call i64 @walk(%p, 4)
    %b = call i64 @walk(%p, 0)
    %c = call i64 @next_used(4)
    %d = call i64 @next_used(0)
    %bad_a = cmp ne i64 %a, 54
    %bad_c = cmp ne i64 %c, 16
    %ab = binary or i64 %bad_a, %b
    %cd = binary or i64 %bad_c, %d
    %bad = binary or i64 %ab, %cd
    return i64 %bad
}
