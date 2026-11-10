# A truncation drops the high half of its source, whatever reads the result.
# Each value here has bits set above its low 32, and the arithmetic does not
# wrap i32, so each widened result is the small number the low halves make.
function @add(%wide : i64, %other : i32) -> i64 [no_inline=yes] {
  block ^entry:
    %low = convert trunc i32 i64 %wide
    %sum = binary add nowrap i32 %low, %other
    %out = convert sext i64 i32 %sum
    return i64 %out
}

function @sub(%wide : i64, %other : i32) -> i64 [no_inline=yes] {
  block ^entry:
    %low = convert trunc i32 i64 %wide
    %difference = binary sub nowrap i32 %low, %other
    %out = convert sext i64 i32 %difference
    return i64 %out
}

function @mul(%wide : i64, %other : i32) -> i64 [no_inline=yes] {
  block ^entry:
    %low = convert trunc i32 i64 %wide
    %product = binary mul nowrap i32 %low, %other
    %out = convert sext i64 i32 %product
    return i64 %out
}

function @element(%base : ptr, %wide : i64, %other : i32) -> i32 [no_inline=yes] {
  block ^entry:
    %low = convert trunc i32 i64 %wide
    %sum = binary add nowrap i32 %low, %other
    %at = index i32 [projection=array_element] %base, %sum
    %value = load typed i32 %at
    return i32 %value
}

function @main() -> i64 [role=entry] {
  slot $values : obj<12x4>

  block ^entry:
    %first = index i32 [projection=array_element] $values, 0
    %second = index i32 [projection=array_element] $values, 1
    %third = index i32 [projection=array_element] $values, 2
    store typed i32 10, %first
    store typed i32 20, %second
    store typed i32 30, %third
    %add = call i64 @add(4294967297, 2)
    %sub = call i64 @sub(8589934597, 7)
    %mul = call i64 @mul(-4294967293, -5)
    %element = call i32 @element(%first, 4294967297, 1)
    %add_bad = cmp ne i64 %add, 3
    %sub_bad = cmp ne i64 %sub, -2
    %mul_bad = cmp ne i64 %mul, -15
    %element_bad = cmp ne i32 %element, 30
    %a = convert zext i64 u8 %add_bad
    %b = convert zext i64 u8 %sub_bad
    %c = convert zext i64 u8 %mul_bad
    %d = convert zext i64 u8 %element_bad
    %ab = binary or i64 %a, %b
    %cd = binary or i64 %c, %d
    %bad = binary or i64 %ab, %cd
    return i64 %bad
}
