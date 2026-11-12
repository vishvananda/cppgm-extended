target wasm32 1

function @observe(%seed : i32) -> i32 {
  slot $input : obj<16x16>
  slot $expected : obj<16x16>
  block ^entry:
    %p0 = addr $input
    %q0 = addr $expected
    %p1 = index i32 %p0, 1
    %q1 = index i32 %q0, 1
    %p2 = index i32 %p0, 2
    %q2 = index i32 %q0, 2
    %p3 = index i32 %p0, 3
    %q3 = index i32 %q0, 3
    store i32 %seed, %p0
    store i32 0x80000001, %p1
    store i32 0x7fa12345, %p2
    store i32 0x01234567, %p3
    %input = load v128 %p0
    %high = vector movshdup %input
    store i32 0x80000001, %q0
    store i32 0x80000001, %q1
    store i32 0x01234567, %q2
    store i32 0x01234567, %q3
    %high_expected = load v128 %q0
    %high_diff = vector pxor %high, %high_expected
    %high_ok = vector ptestz %high_diff, %high_diff
    %low = vector movsldup %input
    store i32 %seed, %q0
    store i32 %seed, %q1
    store i32 0x7fa12345, %q2
    store i32 0x7fa12345, %q3
    %low_expected = load v128 %q0
    %low_diff = vector pxor %low, %low_expected
    %low_ok = vector ptestz %low_diff, %low_diff
    %double = vector movddup %input
    store i32 %seed, %q0
    store i32 0x80000001, %q1
    store i32 %seed, %q2
    store i32 0x80000001, %q3
    %double_expected = load v128 %q0
    %double_diff = vector pxor %double, %double_expected
    %double_ok = vector ptestz %double_diff, %double_diff
    %both = binary and i32 %high_ok, %low_ok
    %all = binary and i32 %both, %double_ok
    return i32 %all
}
