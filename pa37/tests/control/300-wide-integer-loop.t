target wasm32 1

function @observe() -> i32 {
  slot $a : i128
  slot $b : i128
  block ^entry:
    store i128 0x10000000000000001, $a
    store i128 0x20000000000000003, $b
    jump ^loop
  block ^loop:
    %n = phi i32 [^entry: 0, ^loop: %next]
    %next = binary add i32 %n, 1
    %a = load i128 $a
    %b = load i128 $b
    store i128 %a, $b
    store i128 %b, $a
    %again = cmp lt i32 %next, 3
    branch %again, ^loop, ^exit
  block ^exit:
    %first = cmp eq i128 %a, 0x10000000000000001
    %second = cmp eq i128 %b, 0x20000000000000003
    %both = binary and i64 %first, %second
    branch %both, ^pass, ^fail
  block ^pass:
    return i32 17
  block ^fail:
    return i32 1
}
