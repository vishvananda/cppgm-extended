function @f(%x : u32) -> i64 {
  block ^entry:
    %v = convert zext exact i64 u32 %x
    return i64 %v
}
