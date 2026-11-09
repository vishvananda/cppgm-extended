function @main() -> i64 [role=entry] {
  block ^entry:
    %memory = stack_alloc 32
    store volatile i64 73, %memory
    %value = load volatile i64 %memory
    %bad = cmp ne i64 %value, 73
    return i64 %bad
}
