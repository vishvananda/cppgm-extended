function @main() -> i64 {
  block ^entry:
    %control = read_mxcsr
    write_mxcsr %control
    return i64 0
}
