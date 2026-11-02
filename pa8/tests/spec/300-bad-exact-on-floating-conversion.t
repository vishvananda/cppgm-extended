function @f(%x : f64) -> f32 {
  block ^entry:
    %v = convert fptrunc exact f32 f64 %x
    return f32 %v
}
