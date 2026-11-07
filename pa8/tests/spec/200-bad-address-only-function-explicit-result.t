declare function @opaque
function @invoke() -> i64 {
  block ^entry:
    %value = call i64 @opaque(7) as (%arg : i64) -> i32
    return i64 %value
}
