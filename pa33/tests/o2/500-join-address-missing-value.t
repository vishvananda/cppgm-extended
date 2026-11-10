function @select(%condition : u8) -> u8 {
  slot $object : obj<8x4>

  block ^entry:
    %initial = addr $object
    zeroinit 8x4 %initial
    branch %condition, ^yes, ^no

  block ^yes:
    %yes_address = index i8 [projection=field] %initial, 4
    store u8 7, %yes_address
    jump ^merge

  block ^no:
    jump ^merge

  block ^merge:
    %base = addr $object
    %address = index i8 [projection=field] %base, 4
    %result = load u8 %address
    return u8 %result
}
