function @main() -> f32 {
  block ^entry:
    %v = copy f32 nanf(0x400000)
    return f32 %v
}
