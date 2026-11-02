# i32 lanes times a literal power of two: a lane shift, not two products.
function @scale(%x : ptr, %out : ptr, %n : i64) -> void [binding=strong] {
  block ^entry:
    %any = cmp gt i64 %n, 0
    branch %any, ^body, ^done

  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %a = index i32 %x, %i
    %c = load typed i32 %a
    %m = binary mul nowrap i32 %c, 8
    %s = binary add nowrap i32 %m, %c
    %p = index i32 %out, %i
    store typed i32 %s, %p
    %next = binary add nowrap i64 %i, 1
    %more = cmp lt i64 %next, %n
    branch %more, ^body, ^done

  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
