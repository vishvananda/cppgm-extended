global @input : i64 [binding=internal] = 3
global @output : i64 [binding=internal] = 0
global @size : i64 [binding=internal] = 2

function @nested(%rows : i64, %columns : i64) -> void
    [binding=internal, no_inline=yes, unwind=no] {
  block ^entry:
    %first = load volatile i64 @input
    %more = cmp gt i64 %rows, 0
    branch %more, ^outer, ^done

  block ^outer:
    %row = phi i64 [^entry: 0, ^next: %row_next]
    %start = phi i64 [^entry: %first, ^next: %total]
    %a0 = load volatile i64 @input
    %a1 = binary add i64 %a0, %start
    %a2 = binary mul i64 %a1, 3
    store volatile i64 %a2, @output
    %a3 = load volatile i64 @input
    %a4 = binary add i64 %a3, %a2
    store volatile i64 %a4, @output
    %a5 = load volatile i64 @input
    %a6 = binary xor i64 %a5, %a4
    store volatile i64 %a6, @output
    branch %more, ^inner, ^next

  block ^inner:
    %column = phi i64 [^outer: 0, ^inner: %column_next]
    %sum = phi i64 [^outer: %a6, ^inner: %sum_next]
    %b0 = load volatile i64 @input
    %sum_next = binary add i64 %sum, %b0
    %column_next = binary add i64 %column, 1
    %again = cmp lt i64 %column_next, %columns
    branch %again, ^inner, ^next

  block ^next:
    %total = phi i64 [^outer: %a6, ^inner: %sum_next]
    %row_next = binary add i64 %row, 1
    %repeat = cmp lt i64 %row_next, %rows
    branch %repeat, ^outer, ^done

  block ^done:
    %result = phi i64 [^entry: %first, ^next: %total]
    store volatile i64 %result, @output
    return void
}

function @main() -> i32 [role=entry, unwind=no] {
  block ^entry:
    %rows = load volatile i64 @size
    %columns = binary mul i64 %rows, 2
    call void @nested(%rows, %columns)
    %result = load volatile i64 @output
    %bad = cmp ne i64 %result, 125
    return i32 %bad
}
