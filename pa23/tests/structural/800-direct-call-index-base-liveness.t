function @g(%a : ptr, %b : i64) -> ptr {
  block ^entry:
    return ptr %a
}

function @f(%this : ptr, %hash : i64) -> ptr {
  slot $this_slot : ptr
  slot $hash_slot : i64

  block ^entry:
    store ptr %this, $this_slot
    store i64 %hash, $hash_slot
    %base = load ptr $this_slot
    %dst = index i8 [projection=field] %base, 0
    %value = load i64 $hash_slot
    %dead = call ptr @g(%dst, %value)
    return ptr %dead
}

function @main() -> i64 [role=entry] {
  block ^entry:
    return i64 0
}
