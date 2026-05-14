function @g(%x : i64) -> i64 {
  block ^entry:
    return i64 %x
}

function @main() -> i64 {
  block ^entry:
    %0 = const i64 3
    %1 = call i64 @g(%0)
    return i64 %1
}
