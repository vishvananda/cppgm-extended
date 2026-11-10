global @left_values [storage=readonly] = { i32 1 i32 4 i32 7 i32 10 i32 13 i32 16 }
global @right_values [storage=readonly] = { i32 2 i32 3 i32 8 i32 9 i32 14 i32 15 }
global @limit : i64 = 6

# The two cursors advance conditionally; both paths write one chosen value.
function @stream(%left : ptr, %right : ptr, %output : ptr, %length : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 0, ^join: %next_i]
    %j = phi i64 [^entry: 0, ^join: %next_j]
    %k = phi i64 [^entry: 0, ^join: %next_k]
    %more = cmp ult i64 %k, %length
    branch %more, ^read, ^done
  block ^read:
    %a_at = index i32 %left, %i
    %b_at = index i32 %right, %j
    %a = load i32 %a_at
    %b = load i32 %b_at
    %take_a = cmp le i32 %a, %b
    branch %take_a, ^take_left, ^take_right
  block ^take_left:
    %left_index = copy i64 %k
    %left_at = index i32 %output, %left_index
    store i32 %a, %left_at
    %advanced_i = binary add i64 %i, 1
    jump ^join
  block ^take_right:
    %right_index = copy i64 %k
    %right_at = index i32 %output, %right_index
    store i32 %b, %right_at
    %advanced_j = binary add i64 %j, 1
    jump ^join
  block ^join:
    %next_i = phi i64 [^take_left: %advanced_i, ^take_right: %i]
    %next_j = phi i64 [^take_left: %j, ^take_right: %advanced_j]
    %next_k = binary add i64 %k, 1
    jump ^head
  block ^done:
    return void
}

function @stream_reverse(%left : ptr, %right : ptr, %output : ptr, %length : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %i = phi i64 [^entry: 5, ^join: %next_i]
    %j = phi i64 [^entry: 5, ^join: %next_j]
    %k = phi i64 [^entry: 0, ^join: %next_k]
    %more = cmp ult i64 %k, %length
    branch %more, ^read, ^done
  block ^read:
    %a_at = index i32 %left, %i
    %b_at = index i32 %right, %j
    %a = load i32 %a_at
    %b = load i32 %b_at
    %take_a = cmp ge i32 %a, %b
    branch %take_a, ^take_left, ^take_right
  block ^take_left:
    %left_index = copy i64 %k
    %left_at = index i32 %output, %left_index
    store i32 %a, %left_at
    %advanced_i = binary sub i64 %i, 1
    jump ^join
  block ^take_right:
    %right_index = copy i64 %k
    %right_at = index i32 %output, %right_index
    store i32 %b, %right_at
    %advanced_j = binary sub i64 %j, 1
    jump ^join
  block ^join:
    %next_i = phi i64 [^take_left: %advanced_i, ^take_right: %i]
    %next_j = phi i64 [^take_left: %j, ^take_right: %advanced_j]
    %next_k = binary add i64 %k, 1
    jump ^head
  block ^done:
    return void
}

# On the false path, 255 is valid even though the unselected nowrap
# increment would overflow. Speculated work must lose arm-only promises.
function @nowrap_choice(%flag : i1, %output : ptr, %value : u8) -> u8 [no_inline=yes] {
  block ^entry:
    branch %flag, ^yes, ^no
  block ^yes:
    %advanced = binary add nowrap u8 %value, 1
    store i32 19, %output
    jump ^join
  block ^no:
    store i32 23, %output
    jump ^join
  block ^join:
    %next = phi u8 [^yes: %advanced, ^no: %value]
    return u8 %next
}

function @main() -> i32 {
  slot $result : obj<24x4>
  slot $empty : i32
  block ^entry:
    %output = addr $result
    %left = addr @left_values
    %right = addr @right_values
    %length = load volatile i64 @limit
    call void @stream(%left, %right, %output, %length)
    %a = load i32 %output
    %p1 = index i32 %output, 1
    %b = load i32 %p1
    %p2 = index i32 %output, 2
    %c = load i32 %p2
    %p3 = index i32 %output, 3
    %d = load i32 %p3
    %p4 = index i32 %output, 4
    %e = load i32 %p4
    %p5 = index i32 %output, 5
    %f = load i32 %p5
    %bad_a = cmp ne i32 %a, 1
    %bad_b = cmp ne i32 %b, 2
    %bad_c = cmp ne i32 %c, 3
    %bad_d = cmp ne i32 %d, 4
    %bad_e = cmp ne i32 %e, 7
    %bad_f = cmp ne i32 %f, 8
    %ab = binary or i64 %bad_a, %bad_b
    %cd = binary or i64 %bad_c, %bad_d
    %ef = binary or i64 %bad_e, %bad_f
    %abcd = binary or i64 %ab, %cd
    %all = binary or i64 %abcd, %ef
    branch %all, ^fail, ^check_reverse
  block ^check_reverse:
    call void @stream_reverse(%left, %right, %output, %length)
    %r0 = load i32 %output
    %bad_r0 = cmp ne i32 %r0, 16
    %r1 = load i32 %p1
    %bad_r1 = cmp ne i32 %r1, 15
    %r2 = load i32 %p2
    %bad_r2 = cmp ne i32 %r2, 14
    %r3 = load i32 %p3
    %bad_r3 = cmp ne i32 %r3, 13
    %r4 = load i32 %p4
    %bad_r4 = cmp ne i32 %r4, 10
    %r5 = load i32 %p5
    %bad_r5 = cmp ne i32 %r5, 9
    %r_bad1 = binary or i64 %bad_r0, %bad_r1
    %r_bad2 = binary or i64 %r_bad1, %bad_r2
    %r_bad3 = binary or i64 %r_bad2, %bad_r3
    %r_bad4 = binary or i64 %r_bad3, %bad_r4
    %r_bad5 = binary or i64 %r_bad4, %bad_r5
    branch %r_bad5, ^fail, ^check_empty
  block ^check_empty:
    store i32 51, $empty
    %empty = addr $empty
    call void @stream(nullptr, nullptr, %empty, 0)
    %kept = load i32 $empty
    %bad_empty = cmp ne i32 %kept, 51
    branch %bad_empty, ^fail, ^check_wrap
  block ^check_wrap:
    %x = call u8 @nowrap_choice(0, %empty, 255)
    %x_store = load i32 $empty
    %y = call u8 @nowrap_choice(1, %empty, 254)
    %y_store = load i32 $empty
    %bad_x = cmp ne u8 %x, 255
    %bad_y = cmp ne u8 %y, 255
    %bad_x_store = cmp ne i32 %x_store, 23
    %bad_y_store = cmp ne i32 %y_store, 19
    %xy = binary or i64 %bad_x, %bad_y
    %stores = binary or i64 %bad_x_store, %bad_y_store
    %bad = binary or i64 %xy, %stores
    %status = convert trunc i32 i64 %bad
    return i32 %status
  block ^fail:
    return i32 1
}
