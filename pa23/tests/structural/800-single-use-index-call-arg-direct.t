function @g(%a : ptr, %b : ptr) -> ptr {
  block ^entry:
    return ptr %a
}

function @f(%this : ptr, %rhs : ptr) -> ptr {
  block ^entry:
    %dst = index ptr %this, 16
    %src = index ptr %rhs, 16
    %dead = call ptr @g(%dst, %src)
    return ptr %this
}

function @main() -> i64 [role=entry] {
  block ^entry:
    return i64 0
}
