function @allocate(%a : i64, %b : i64, %c : i64,
                   %d : i64, %e : i64, %f : i64, %bytes : i64) -> i64 {
  block ^entry:
    %memory = stack_alloc %bytes
    store volatile i64 73, %memory
    %value = load i64 %memory
    return i64 %value
}
function @main() -> i64 [role=entry] {
  block ^entry:
    %actual = call i64 @allocate(1, 2, 3, 4, 5, 6, 32)
    %bad = cmp ne i64 %actual, 73
    return i64 %bad
}
