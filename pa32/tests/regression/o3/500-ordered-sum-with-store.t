function @sum_and_store(%x : ptr, %d : ptr, %n : i64) -> f64 [no_inline=yes] {
  block ^entry:
    jump ^header

  block ^header:
    %i = phi i64 [^entry: 0, ^latch: %next]
    %s = phi f64 [^entry: 0, ^latch: %sum]
    %more = cmp lt i64 %i, %n
    branch %more, ^latch, ^exit

  block ^latch:
    %at = index f64 %x, %i
    %v = load typed f64 %at
    %to = index f64 %d, %i
    store typed f64 0.25, %to
    %sum = binary add f64 %s, %v
    %next = binary add i64 %i, 1
    jump ^header

  block ^exit:
    return f64 %s
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
