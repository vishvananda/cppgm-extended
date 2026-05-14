function @g(%a : ptr) -> ptr {
  block ^entry:
    return ptr %a
}

function @f(%a : ptr, %b : ptr, %c : ptr, %src : ptr) -> ptr {
  slot $tmp : obj<8x8>

  block ^entry:
    %keep = copy ptr %src
    %dst = addr $tmp
    copyobj 8x8 %src, %dst
    %r = call ptr @g(%keep)
    return ptr %r
}

function @main() -> i64 [role=entry] {
  block ^entry:
    return i64 0
}
