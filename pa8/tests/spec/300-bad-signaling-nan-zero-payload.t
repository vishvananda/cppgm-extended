function @main() -> f64 {
  block ^entry:
    %v = copy f64 snan(0x0)
    return f64 %v
}
