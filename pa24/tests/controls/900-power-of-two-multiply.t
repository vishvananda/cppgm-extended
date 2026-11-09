global @input = {
  i64 7
}

function @main() -> i64 [role=entry, unwind=no] {
  block ^entry:
    %address = addr @input
    %value = load volatile i64 %address
    %result = binary mul i64 %value, 8
    return i64 %result
}
