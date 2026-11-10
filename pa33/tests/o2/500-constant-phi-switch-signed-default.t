function @select(%condition : u8) -> i64 {
  block ^entry:
    branch %condition, ^negative, ^other

  block ^negative:
    jump ^dispatch

  block ^other:
    jump ^dispatch

  block ^dispatch:
    %tag = phi u8 [^negative: 255, ^other: 12]
    %selector = convert sext i64 u8 %tag
    switch %selector, ^fallback, -1:^hit, 3:^unused, 4:^hit

  block ^hit:
    %selected = phi i64 [^dispatch: 19]
    return i64 %selected

  block ^unused:
    return i64 31

  block ^fallback:
    return i64 23
}
