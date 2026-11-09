global @address : ptr [binding=internal] = 0
global @total : i64 [binding=internal] = 0
global @flag : i64 [binding=internal] = 1
function @probe() -> void [binding=internal, no_inline=yes, unwind=no] {
  slot $dead : ptr
  block ^entry:
    store ptr @total, $dead
    %flag = load volatile i64 @flag
    %skip = cmp eq i64 %flag, 0
    branch %skip, ^done, ^check
  block ^check:
    %skip_again = cmp eq i64 %flag, 2
    branch %skip_again, ^done, ^check_observe
  block ^check_observe:
    %see = cmp eq i64 %flag, 99
    branch unlikely %see, ^observe, ^work
  block ^work:
    %slow = cmp eq i64 %flag, 1
    branch unlikely %slow, ^cold, ^join
  block ^join:
    store ptr 0, $dead
    jump ^done
  block ^done:
    return void
  block ^observe:
    %pointer = load ptr $dead
    store volatile ptr %pointer, @address
    jump ^done
  block ^cold:
    %v0 = load volatile i64 @total
    %w0 = binary add i64 %v0, 1
    store volatile i64 %w0, @total
    %v1 = load volatile i64 @total
    %w1 = binary add i64 %v1, 1
    store volatile i64 %w1, @total
    %v2 = load volatile i64 @total
    %w2 = binary add i64 %v2, 1
    store volatile i64 %w2, @total
    %v3 = load volatile i64 @total
    %w3 = binary add i64 %v3, 1
    store volatile i64 %w3, @total
    %v4 = load volatile i64 @total
    %w4 = binary add i64 %v4, 1
    store volatile i64 %w4, @total
    %v5 = load volatile i64 @total
    %w5 = binary add i64 %v5, 1
    store volatile i64 %w5, @total
    jump ^join
}
function @main() -> i64 [role=entry, unwind=no] {
  block ^entry:
    store volatile i64 0, @flag
    call void @probe()
    store volatile i64 2, @flag
    call void @probe()
    store volatile i64 99, @flag
    call void @probe()
    store volatile i64 1, @flag
    call void @probe()
    %value = load volatile i64 @total
    %bad_value = cmp ne i64 %value, 6
    %address = load volatile ptr @address
    %bad_address = cmp ne ptr %address, @total
    %bad = binary or i64 %bad_value, %bad_address
    return i64 %bad
}
