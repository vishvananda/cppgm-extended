# Fixed fields and indexed payload bytes must remain coherent across snapshots.
function @scan(%n : i32, %bias : i32) -> i32 [no_inline=yes] {
  slot $current : obj<520x8>
  slot $snapshot : obj<520x8>
  block ^entry:
    %p = addr $current
    %q = addr $snapshot
    %payload = index i8 %p, 8
    zeroinit 520x8 %p
    store i32 23, %payload
    jump ^test
  block ^test:
    %i = phi i32 [^entry: 0, ^body: %next]
    %more = cmp lt i32 %i, %n
    branch %more, ^body, ^done
  block ^body:
    %count = load i32 %p
    %sum = binary add i32 %count, %bias
    store i32 %sum, %p
    %index = binary and i32 %i, 127
    %item = index i32 %payload, %index
    store i32 %i, %item
    %next = binary add i32 %i, 1
    jump ^test
  block ^done:
    copyobj 520x8 %p, %q
    store i32 -123, %p
    store i32 -9, %payload
    %saved_count = load i32 %q
    %saved_data = index i8 %q, 8
    %head = load i32 %saved_data
    %last = binary sub i32 %n, 1
    %last_index = binary and i32 %last, 127
    %last_item = index i32 %saved_data, %last_index
    %tail = load i32 %last_item
    %subtotal = binary add i32 %saved_count, %head
    %result = binary add i32 %subtotal, %tail
    return i32 %result
}
function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i32 @scan(4, 3)
    %b = call i32 @scan(0, 5)
    %c = call i32 @scan(130, 3)
    %bad_a = cmp ne i32 %a, 15
    %bad_b = cmp ne i32 %b, 23
    %bad_c = cmp ne i32 %c, 647
    %ab = binary or i1 %bad_a, %bad_b
    %bad = binary or i1 %ab, %bad_c
    %out = convert zext i32 i1 %bad
    return i32 %out
}
