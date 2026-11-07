declare function @opaque
function @invoke() -> i64 {
  block ^entry:
    %pointer = const ptr null
    %value = call i64 @opaque(%pointer) as (%arg : i64) -> i64
    return i64 %value
}
