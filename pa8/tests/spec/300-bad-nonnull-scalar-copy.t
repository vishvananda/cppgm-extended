function @bad(%x : i64) -> i64 {
  block ^entry:
    %v = copy nonnull i64 %x
    return i64 %v
}
