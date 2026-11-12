global @counter : i64 [cold=yes] = 0

function @main() -> i64 [role=entry] {
  block ^entry:
    return i64 0
}
