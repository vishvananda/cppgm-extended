target wasm32 1

function @observe() -> i32 {
  slot $area : obj<32x16>
  block ^entry:
    store i128 0x112233445566778899aabbcc12345678, $area
    %base = addr $area
    %middle = index i64 %base, 1
    store i128 $area, %middle
    %low = load i64 %middle
    %upper = index i64 %middle, 1
    %high = load i64 %upper
    %a = cmp eq i64 %low, 0x99aabbcc12345678
    %b = cmp eq i64 %high, 0x1122334455667788
    %bits = load i128 %middle
    %pointer = copy ptr %bits
    %word = copy u32 %pointer
    %c = cmp eq u32 %word, 0x12345678
    %ab = binary and i64 %a, %b
    %abc = binary and i64 %ab, %c
    branch %abc, ^pass, ^fail
  block ^pass:
    return i32 17
  block ^fail:
    return i32 1
}
