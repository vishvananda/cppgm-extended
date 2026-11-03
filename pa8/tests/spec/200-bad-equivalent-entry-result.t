function @a(%x : i64) -> i64 {
  block ^entry:
    return i64 %x
}
function @b(%x : i64) -> void {
  block ^entry:
    return void
}
equivalent_entry @a = @b
