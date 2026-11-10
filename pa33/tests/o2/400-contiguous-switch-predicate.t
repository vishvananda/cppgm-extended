# Case membership preserves narrow widths, signed limits, holes and wide selectors.

function @interval_i8_low(%x : i8, %count : i32) -> i32 [no_inline=yes] {
  block ^entry:
    switch %x, ^miss, 1:^hit, 2:^hit, 3:^hit
  block ^hit:
    %next = binary add nowrap i32 %count, 1
    jump ^join
  block ^miss:
    jump ^join
  block ^join:
    %result = phi i32 [^hit: %next, ^miss: %count]
    return i32 %result
}

function @interval_u8_sign(%x : u8, %count : i32) -> i32 [no_inline=yes] {
  block ^entry:
    switch %x, ^miss, 127:^hit, 128:^hit, 129:^hit
  block ^hit:
    %next = binary add nowrap i32 %count, 1
    jump ^join
  block ^miss:
    jump ^join
  block ^join:
    %result = phi i32 [^hit: %next, ^miss: %count]
    return i32 %result
}

function @interval_i32_sign(%x : i32, %count : i32) -> i32 [no_inline=yes] {
  block ^entry:
    switch %x, ^miss, 2147483647:^hit, 2147483648:^hit, 2147483649:^hit
  block ^hit:
    %next = binary add nowrap i32 %count, 1
    jump ^join
  block ^miss:
    jump ^join
  block ^join:
    %result = phi i32 [^hit: %next, ^miss: %count]
    return i32 %result
}

function @interval_i64_high(%x : i64, %count : i32) -> i32 [no_inline=yes] {
  block ^entry:
    switch %x, ^miss, -3:^hit, -2:^hit, -1:^hit
  block ^hit:
    %next = binary add nowrap i32 %count, 1
    jump ^join
  block ^miss:
    jump ^join
  block ^join:
    %result = phi i32 [^hit: %next, ^miss: %count]
    return i32 %result
}

function @interval_i64_cyclic(%x : i64, %count : i32) -> i32 [no_inline=yes] {
  block ^entry:
    switch %x, ^miss, -1:^hit, 0:^hit, 1:^hit
  block ^hit:
    %next = binary add nowrap i32 %count, 1
    jump ^join
  block ^miss:
    jump ^join
  block ^join:
    %result = phi i32 [^hit: %next, ^miss: %count]
    return i32 %result
}

function @interval_i16_sparse(%x : i16, %count : i32) -> i32 [no_inline=yes] {
  block ^entry:
    switch %x, ^miss, 1:^hit, 3:^hit, 4:^hit
  block ^hit:
    %next = binary add nowrap i32 %count, 1
    jump ^join
  block ^miss:
    jump ^join
  block ^join:
    %result = phi i32 [^hit: %next, ^miss: %count]
    return i32 %result
}

function @interval_u16_unsorted(%x : u16, %count : i32) -> i32 [no_inline=yes] {
  block ^entry:
    switch %x, ^miss, 19:^hit, 17:^hit, 18:^hit
  block ^hit:
    %next = binary add nowrap i32 %count, 1
    jump ^join
  block ^miss:
    jump ^join
  block ^join:
    %result = phi i32 [^hit: %next, ^miss: %count]
    return i32 %result
}

function @wide_selector(%x : i128, %count : i32) -> i32 [no_inline=yes] {
  block ^entry:
    switch %x, ^miss, 1:^hit, 2:^hit, 3:^hit
  block ^hit:
    %next = binary add nowrap i32 %count, 1
    jump ^join
  block ^miss:
    jump ^join
  block ^join:
    %result = phi i32 [^hit: %next, ^miss: %count]
    return i32 %result
}

function @singleton(%x : i32) -> i32 [no_inline=yes] {
  block ^entry:
    switch %x, ^miss, -1:^hit
  block ^hit:
    return i32 1
  block ^miss:
    return i32 0
}
