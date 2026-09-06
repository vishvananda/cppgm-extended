function @main(%value : i64) -> i64 [role=entry] {
  block ^entry:
    %negated = neg i64 %value
    return i64 %negated
}
