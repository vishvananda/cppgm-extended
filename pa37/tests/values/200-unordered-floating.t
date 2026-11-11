target wasm32 1

function @unordered(%x : f64, %y : f64) -> i64 {
  block ^entry:
    %a = cmp ult f64 %x, %y
    %b = cmp ule f64 %x, %y
    %c = cmp ugt f64 %x, %y
    %d = cmp uge f64 %x, %y
    %ab = binary add i64 %a, %b
    %cd = binary add i64 %c, %d
    %result = binary add i64 %ab, %cd
    return i64 %result
}
