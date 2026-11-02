# i32 lanes beside f64 lanes: two lanes at the bottom of the register move to
# the even lanes for one product.
function @fill(%x : ptr, %y : ptr, %out : ptr, %n : i64) -> void [binding=strong] {
  block ^entry:
    %any = cmp gt i64 %n, 0
    branch %any, ^body, ^done

  block ^body:
    %i = phi i64 [^entry: 0, ^body: %next]
    %a = index i32 %x, %i
    %b = index i32 %y, %i
    %c = load typed i32 %a
    %d = load typed i32 %b
    %m = binary mul nowrap i32 %c, %d
    %f = convert sitofp f64 i32 %m
    %o = index f64 %out, %i
    store typed f64 %f, %o
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
