function @a(%x : i64) -> i64 {
  block ^entry:
    return i64 %x
}
function @b(%x : i64) -> i64 {
  block ^entry:
    return i64 %x
}
declare function @c(%x : i64) -> i64
equivalent_entry @a = @b
equivalent_entry @b = @c
