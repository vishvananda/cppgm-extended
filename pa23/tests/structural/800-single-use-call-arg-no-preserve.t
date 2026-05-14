function @one() -> i64 {
  block ^entry:
    return i64 1
}

function @foo(%a : i64, %b : i64, %c : i64, %d : i64, %e : i64, %f : i64) -> i64 {
  block ^entry:
    return i64 0
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %x = call i64 @one()
    %dead = call i64 @foo(%x, 20, 30, 40, 50, 60)
    return i64 0
}
