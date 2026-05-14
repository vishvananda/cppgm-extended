function @main() -> i64 {
  block ^entry:
    %x = unary decay i64 1
    return i64 %x
}
