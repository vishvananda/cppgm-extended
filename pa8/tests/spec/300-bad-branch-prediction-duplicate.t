function @main() -> i64 {
  block ^entry:
    branch likely unlikely 0, ^yes, ^no
  block ^yes:
    return i64 1
  block ^no:
    return i64 0
}
