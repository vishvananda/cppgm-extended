function @pair(%low : i64, %high : i64) -> i128 {
  block ^entry:
    %a = convert zext i128 i64 %low
    %b = convert zext i128 i64 %high
    %upper = binary shl i128 %b, 64
    %out = binary or i128 %a, %upper
    return i128 %out
}

function @parts(%value : i128) -> i64 {
  block ^entry:
    %low = convert trunc i64 i128 %value
    %shifted = binary ushr i128 %value, 64
    %high = convert trunc i64 i128 %shifted
    %out = binary xor i64 %low, %high
    return i64 %out
}

function @stack_parts(%a : i64, %b : i64, %c : i64, %d : i64, %e : i64, %f : i64, %value : i128) -> i64 {
  block ^entry:
    %low = convert trunc i64 i128 %value
    %shifted = binary ushr i128 %value, 64
    %high = convert trunc i64 i128 %shifted
    %out = binary xor i64 %low, %high
    %all = binary add i64 %out, %f
    return i64 %all
}

function @low_only(%value : i128) -> i64 {
  block ^entry:
    %low = convert trunc i64 i128 %value
    return i64 %low
}

function @call_low_only(%low : i64, %high : i64) -> i64 {
  block ^entry:
    %value = call i128 @pair(%low, %high)
    %out = convert trunc i64 i128 %value
    return i64 %out
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %value = call i128 @pair(305419896, 4275878552)
    %low = convert trunc i64 i128 %value
    %shifted = binary ushr i128 %value, 64
    %high = convert trunc i64 i128 %shifted
    %first = cmp ne i64 %low, 305419896
    %second = cmp ne i64 %high, 4275878552
    %got = call i64 @parts(%value)
    %expected = binary xor i64 %low, %high
    %third = cmp ne i64 %got, %expected
    %stack = call i64 @stack_parts(1, 2, 3, 4, 5, 6, %value)
    %expected_stack = binary add i64 %expected, 6
    %fourth = cmp ne i64 %stack, %expected_stack
    %ab = binary or i64 %first, %second
    %cd = binary or i64 %third, %fourth
    %bad = binary or i64 %ab, %cd
    %lone = call i64 @low_only(%value)
    %fifth = cmp ne i64 %lone, 305419896
    %call = call i64 @call_low_only(305419896, 4275878552)
    %sixth = cmp ne i64 %call, 305419896
    %ef = binary or i64 %fifth, %sixth
    %all_bad = binary or i64 %bad, %ef
    return i64 %all_bad
}
