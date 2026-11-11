function @f(%p : ptr) -> i64 {
  block ^entry:
    %b = load typed i1 %p
    %w = convert zext i64 i1 %b
    return i64 %w
}
