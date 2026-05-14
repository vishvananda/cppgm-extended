function @f(%x : i64) -> i64 {
  slot $y : i64

  block ^entry:
    %t1 = copy i64 %x
    store i64 %t1, $y
    %t2 = copy i64 %t1
    return i64 %t2
}

function @__cppgm_406d61696e() -> i32 [role=entry, binding=strong] {
  block ^entry:
    %x = const i64 41
    %y = call i64 @f(%x)
    return i32 0
}
