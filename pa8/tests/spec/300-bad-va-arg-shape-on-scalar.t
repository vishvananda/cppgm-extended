function @bad(%list : ptr) -> i32 {
  block ^entry:
    %value = va_arg i32 %list [shape=i32x1]
    return i32 %value
}
