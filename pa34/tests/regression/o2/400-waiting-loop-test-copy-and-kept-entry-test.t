global @list : obj<32x4> [binding=internal] = {
  zero 32
}
global @top : i32 [binding=internal] = zero

function @sort() -> void [binding=internal, no_inline=yes] {
  block ^entry:
    store typed i32 7, @top
    %base = addr @list
    jump ^test

  block ^test:
    %limit = phi i32 [^entry: 7, ^next: %limit_next]
    %i = phi i32 [^entry: 1, ^next: %i_next]
    %more = cmp lt i32 %i, %limit
    branch %more, ^body, ^pass_done

  block ^body:
    %left_address = index inbounds i32 [projection=array_element] %base, %i
    %left = load typed i32 %left_address
    %j = binary add nowrap i32 %i, 1
    %right_address = index inbounds i32 [projection=array_element] %base, %j
    %right = load typed i32 %right_address
    %unordered = cmp gt i32 %left, %right
    branch %unordered, ^swap, ^next

  block ^swap:
    store typed i32 %right, %left_address
    store typed i32 %left, %right_address
    jump ^next

  block ^pass_done:
    %shorter = binary sub nowrap i32 %limit, 1
    store typed i32 %shorter, @top
    %again = cmp gt i32 %shorter, 1
    branch %again, ^next, ^done

  block ^done:
    return void

  block ^next:
    %limit_next = phi i32 [^body: %limit, ^swap: %limit, ^pass_done: %shorter]
    %i_next = phi i32 [^pass_done: 1, ^body: %j, ^swap: %j]
    jump ^test
}

function @mix(%a : i32, %b : i32) -> i32 [binding=internal, no_inline=yes] {
  block ^entry:
    %sum = binary add i32 %a, %b
    %low = binary and i32 %sum, 3
    return i32 %low
}

function @walk(%m : i32, %n : i32) -> i32 [binding=internal, no_inline=yes] {
  block ^entry:
    %start = binary add nowrap i32 %m, 1
    jump ^outer

  block ^outer:
    %m_now = phi i32 [^step: %m_next, ^entry: %start]
    %n_now = phi i32 [^step: %n_next, ^entry: %n]
    branch %n_now, ^inner, ^step

  block ^inner:
    %n_less = binary sub nowrap i32 %n_now, 1
    %mixed = call i32 @mix(%m_now, %n_less)
    jump ^step

  block ^step:
    %n_next = phi i32 [^inner: %mixed, ^outer: 1]
    %m_next = binary sub nowrap i32 %m_now, 1
    branch %m_next, ^outer, ^last

  block ^last:
    %result = binary add nowrap i32 %n_next, 1
    %odd = binary and i32 %result, 1
    branch %odd, ^bump, ^done

  block ^bump:
    %more = call i32 @mix(%result, 7)
    jump ^done

  block ^done:
    %out = phi i32 [^last: %result, ^bump: %more]
    return i32 %out
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %base = addr @list
    jump ^fill

  block ^fill:
    %k = phi i32 [^entry: 0, ^fill: %k_next]
    %value = binary sub nowrap i32 8, %k
    %cell = index inbounds i32 [projection=array_element] %base, %k
    store typed i32 %value, %cell
    %k_next = binary add nowrap i32 %k, 1
    %filling = cmp lt i32 %k_next, 8
    branch %filling, ^fill, ^run

  block ^run:
    call void @sort()
    %second_address = index inbounds i32 [projection=array_element] %base, 2
    %second = load typed i32 %second_address
    %a = call i32 @walk(5, 3)
    %sum = binary add i32 %second, %a
    %wide = convert sext i64 i32 %sum
    return i64 %wide
}
