# Direct slot bases and temporary addresses describe the same private fields.
function @mixed(%first : f64, %second : f64, %bits : i64, %choose : i1) -> f64 [no_inline=yes] {
  slot $pair : obj<16x8>
  block ^entry:
    %base = addr $pair
    %high = index i8 %base, 8
    store f64 %first, %base
    store f64 %second, %high
    branch %choose, ^replace, ^read
  block ^replace:
    %direct = index i8 $pair, 8
    store i64 %bits, %direct
    jump ^read
  block ^read:
    %a = load f64 %base
    %b = load f64 %high
    %sum = binary add f64 %a, %b
    return f64 %sum
}

function @mutate(%where : ptr) -> void [no_inline=yes] {
  block ^entry:
    store f64 7.0, %where
    return void
}

function @escaped(%first : f64) -> f64 [no_inline=yes] {
  slot $pair : obj<16x8>
  block ^entry:
    store f64 %first, $pair
    %direct = index i8 $pair, 8
    store f64 2.0, %direct
    call void @mutate(%direct)
    %a = load f64 $pair
    %b = load f64 %direct
    %sum = binary add f64 %a, %b
    return f64 %sum
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call f64 @mixed(3.0, 5.0, 4618441417868443648, 0)
    %b = call f64 @mixed(3.0, 5.0, 4618441417868443648, 1)
    %c = call f64 @escaped(3.0)
    %x = cmp eq f64 %a, 8.0
    %y = cmp eq f64 %b, 9.0
    %z = cmp eq f64 %c, 10.0
    %xy = binary and i64 %x, %y
    %ok = binary and i64 %xy, %z
    branch %ok, ^passed, ^failed
  block ^passed:
    return i32 0
  block ^failed:
    return i32 1
}
