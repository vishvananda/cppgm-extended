function @select(%x : u32, %present : u8) -> u32 {
  slot $retobj__1 : obj<8x4>

  block ^entry:
    branch %present, ^if_then_1, ^if_end_2

  block ^if_then_1:
    %t1 = addr $retobj__1
    store u32 %x, %t1
    %t2 = index i8 [projection=field] %t1, 4
    store u8 1, %t2
    jump ^inline_cont_3

  block ^if_end_2:
    %t3 = addr $retobj__1
    zeroinit 1x1 %t3
    %t4 = index i8 [projection=field] %t3, 4
    store u8 0, %t4
    %t5 = load u32 %t3
    jump ^inline_cont_3

  block ^inline_cont_3:
    %t6 = phi u32 [^if_then_1: %x, ^if_end_2: %t5]
    %t7 = addr $retobj__1
    %t8 = index i8 [projection=field] %t7, 4
    %t9 = load u8 %t8
    %t10 = select u32 %t9, %t6, 3
    return u32 %t10
}
