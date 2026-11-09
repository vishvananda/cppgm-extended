global @input = {
  i64 -1
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %address = addr @input
    %value = load volatile i64 %address
    %low = binary and i64 %value, 3
    %high = binary and i64 %value, 4294967299
    %low_bad = cmp ne i64 %low, 3
    %high_bad = cmp ne i64 %high, 4294967299
    %bad = binary or i64 %low_bad, %high_bad
    %exit = convert trunc i32 i64 %bad
    return i32 %exit
}
