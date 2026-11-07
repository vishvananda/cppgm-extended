declare function @opaque
function @invoke() -> i64 {
  block ^entry:
    %value = call i64 @opaque(7)
    return i64 %value
}
