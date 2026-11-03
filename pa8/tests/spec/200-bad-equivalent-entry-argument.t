function @a(%x : i64) -> i64 {
  block ^entry:
    return i64 %x
}
function @b(%x : i32) -> i64 {
  block ^entry:
    return i64 0
}
equivalent_entry @a = @b
