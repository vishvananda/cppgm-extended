function @a(%x : i64) -> i64 {
  block ^entry:
    return i64 %x
}
function @b(%x : i64, ...) -> i64 {
  block ^entry:
    return i64 %x
}
equivalent_entry @a = @b
