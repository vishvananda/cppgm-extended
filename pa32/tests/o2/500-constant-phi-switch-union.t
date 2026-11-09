function @select(%value : u32, %wide : u8) -> i64 {
  slot $payload : obj<8x8>

  block ^entry:
    %address = addr $payload
    branch %wide, ^wide, ^narrow

  block ^wide:
    %extended = convert zext i64 u32 %value
    store typed i64 %extended, %address
    jump ^visit

  block ^narrow:
    store typed u32 %value, %address
    jump ^visit

  block ^visit:
    %tag = phi u8 [^wide: 1, ^narrow: 0]
    %selector = convert zext i64 u8 %tag
    switch %selector, ^invalid, 0:^read_narrow, 1:^read_wide

  block ^read_narrow:
    %small = load typed u32 %address
    %small_extended = convert zext i64 u32 %small
    %small_result = binary add i64 %small_extended, 7
    jump ^done

  block ^read_wide:
    %large = load typed i64 %address
    %large_result = binary add i64 %large, 7
    jump ^done

  block ^invalid:
    unreachable

  block ^done:
    %result = phi i64 [^read_narrow: %small_result, ^read_wide: %large_result]
    return i64 %result
}
