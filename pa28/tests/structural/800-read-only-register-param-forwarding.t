function @g(%this : ptr, %__i : i64) -> ptr {
  slot $this : ptr
  slot $__i : i64

  block ^entry:
    store ptr %this, $this
    store i64 %__i, $__i
    %t1 = load ptr $this
    %t2 = index i8 [projection=field] %t1, 0
    %t3 = load ptr %t2
    %t4 = load i64 $__i
    %t5 = index ptr [projection=array_element] %t3, %t4
    return ptr %t5
}

function @main() -> i64 [role=entry] {
  block ^entry:
    return i64 0
}
