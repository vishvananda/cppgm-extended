function @choose(%value : i64) -> i64 {
  block ^entry:
    branch likely %value, ^nonzero, ^zero
  block ^nonzero:
    return i64 7
  block ^zero:
    return i64 11
}

function @main() -> i64 {
  block ^entry:
    %a = call i64 @choose(0)
    %b = call i64 @choose(1)
    %ok = cmp eq i64 %a, 11
    branch unlikely %ok, ^check, ^bad
  block ^check:
    %other = cmp eq i64 %b, 7
    branch %other, ^good, ^bad
  block ^good:
    return i64 0
  block ^bad:
    return i64 1
}
