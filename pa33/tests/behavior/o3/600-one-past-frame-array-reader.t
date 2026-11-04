function @backwards(%end : ptr, %count : i64) -> i64 {
  block ^entry:
    jump ^test
  block ^test:
    %p = phi ptr [^entry: %end, ^body: %prev]
    %n = phi i64 [^entry: %count, ^body: %next]
    %sum = phi i64 [^entry: 0, ^body: %added]
    branch %n, ^body, ^done
  block ^body:
    %prev = index i64 %p, -1
    %value = load i64 %prev
    %added = binary add i64 %sum, %value
    %next = binary sub i64 %n, 1
    jump ^test
  block ^done:
    return i64 %sum
}
function @adjacent_array() -> i64 {
  slot $neighbor : i64
  slot $values : obj<80x8>
  block ^entry:
    store i64 71, $neighbor
    %base = addr $values
    zeroinit 80x8 %base
    store i64 55, %base
    %end = index i64 %base, 10
    %sum = call i64 @backwards(%end, 10)
    %neighbor = load i64 $neighbor
    %total = binary add i64 %sum, %neighbor
    return i64 %total
}
function @main() -> i32 [role=entry] {
  slot $values : obj<80x8>
  block ^entry:
    %base = addr $values
    zeroinit 80x8 %base
    store i64 55, %base
    %end = index i64 %base, 10
    %sum = call i64 @backwards(%end, 10)
    %wrong = cmp ne i64 %sum, 55
    %result = convert trunc i32 i64 %wrong
    %adjacent = call i64 @adjacent_array()
    %bad_adjacent = cmp ne i64 %adjacent, 126
    %combined = binary or i32 %result, %bad_adjacent
    return i32 %combined
}
