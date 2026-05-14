function @id(%a : i64) -> i64 {
  block ^entry:
    return i64 %a
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %x = const i64 7
    %dead = call i64 @id(%x)
    return i64 %x
}
