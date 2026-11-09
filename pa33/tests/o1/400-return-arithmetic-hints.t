function @sum64(%a : i64, %b : i64) -> i64 {
  block ^entry:
    %sum = binary add i64 %a, %b
    return i64 %sum
}

function @step64(%a : i64) -> i64 {
  block ^entry:
    %sum = binary add i64 %a, 3
    return i64 %sum
}

function @sum32(%a : u32, %b : u32) -> u32 {
  block ^entry:
    %sum = binary add u32 %a, %b
    return u32 %sum
}

function @main() -> i64 {
  block ^entry:
    %wide = call i64 @sum64(9223372036854775807, 1)
    %wide_ok = cmp eq i64 %wide, -9223372036854775808
    branch %wide_ok, ^step, ^bad

  block ^step:
    %incremented = call i64 @step64(-3)
    %step_ok = cmp eq i64 %incremented, 0
    branch %step_ok, ^narrow, ^bad

  block ^narrow:
    %small = call u32 @sum32(4294967295, 2)
    %small_ok = cmp eq u32 %small, 1
    branch %small_ok, ^good, ^bad

  block ^good:
    return i64 0

  block ^bad:
    return i64 1
}
