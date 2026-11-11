# Floating fields retain their representation, including NaN payloads and -0.
function @floats(%lo : f32, %hi : f32) -> obj<8x4> [no_inline=yes] {
  slot $pair : obj<8x4>
  block ^entry:
    %p = addr $pair
    %q = index i8 %p, 4
    store f32 %lo, %p
    store f32 %hi, %q
    return obj<8x4> $pair
}

function @read_float_words(%bits : i64) -> i64 [no_inline=yes] {
  slot $pair : obj<8x4>
  block ^entry:
    store i64 %bits, $pair
    %p = addr $pair
    %lo = load f32 %p
    %q = index i8 %p, 4
    %hi = load f32 %q
    %low = copy u32 %lo
    %high = copy u32 %hi
    %a = convert zext i64 u32 %low
    %b = convert zext i64 u32 %high
    %shift = binary shl i64 %b, 32
    %result = binary or i64 %a, %shift
    return i64 %result
}

function @double_word(%bits : i64) -> i64 [no_inline=yes] {
  slot $cell : obj<8x8>
  block ^entry:
    %value = copy f64 %bits
    %p = addr $cell
    store f64 %value, %p
    %read = load i64 %p
    return i64 %read
}

function @main() -> i32 [role=entry] {
  slot $result : obj<8x4>
  block ^entry:
    %low_bits = copy u32 2147483648
    %high_bits = copy u32 2143363909
    %lo = copy f32 %low_bits
    %hi = copy f32 %high_bits
    %value = call obj<8x4> @floats(%lo, %hi)
    copyobj 8x4 %value, $result
    %p = addr $result
    %bits = load i64 %p
    %a = cmp eq i64 %bits, 9205677894729203712
    %roundtrip = call i64 @read_float_words(%bits)
    %b = cmp eq i64 %roundtrip, %bits
    %double = call i64 @double_word(9221365074855185357)
    %c = cmp eq i64 %double, 9221365074855185357
    %ab = binary and i64 %a, %b
    %ok = binary and i64 %ab, %c
    branch %ok, ^passed, ^failed
  block ^passed:
    return i32 0
  block ^failed:
    return i32 1
}
