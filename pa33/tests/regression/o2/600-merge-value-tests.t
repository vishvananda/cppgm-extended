# Two tests of one value against literals become one test where one says the
# same: bits of either mask set, a run of consecutive values, two values one
# bit apart. The second test sits alone in a block that shares a target with
# the first, or the two meet in a select: of two compares, or of one value
# and a select of the same value.
global @seen : i64 = 0
global @spare : i64 = 0

function @note(%v : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return void
}

# (x & 4) || (x & 16): one mask.
function @either_bit(%x : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and i32 %x, 4
    branch %a, ^yes, ^more

  block ^more:
    %b = binary and i32 %x, 16
    %c = cmp ne i32 %b, 0
    branch %c, ^yes, ^no

  block ^yes:
    call void @note(1)
    return void

  block ^no:
    call void @note(2)
    return void
}

# (x & 4) == 0 && (x & 16) == 0: the same mask, the other way.
function @both_clear(%x : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and i32 %x, 4
    %c = cmp eq i32 %a, 0
    branch %c, ^more, ^no

  block ^more:
    %b = binary and i32 %x, 16
    %d = cmp eq i32 %b, 0
    branch %d, ^yes, ^no

  block ^yes:
    call void @note(3)
    return void

  block ^no:
    call void @note(4)
    return void
}

# 10 <= x && x <= 20: x - 10 <= 10, unsigned.
function @range(%x : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp ge i32 %x, 10
    branch %a, ^more, ^no

  block ^more:
    %b = cmp le i32 %x, 20
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(5)
    return void

  block ^no:
    call void @note(6)
    return void
}

# x == 5 || x == 6 || x == 7: a run, grown twice.
function @run_of_three(%x : u8) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq u8 %x, 5
    branch %a, ^yes, ^second

  block ^second:
    %b = cmp eq u8 %x, 6
    branch %b, ^yes, ^third

  block ^third:
    %c = cmp eq u8 %x, 7
    branch %c, ^yes, ^no

  block ^yes:
    call void @note(7)
    return void

  block ^no:
    call void @note(8)
    return void
}

# x == 8 || x == 12: the value less the bit that tells them apart.
function @one_bit_apart(%x : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i64 %x, 8
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp eq i64 %x, 12
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(9)
    return void

  block ^no:
    call void @note(10)
    return void
}

# x != 0 && x != -1 through a stepped value: x + 1 > 1, unsigned.
function @neither_end(%x : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp ne i64 %x, 0
    branch %a, ^more, ^no

  block ^more:
    %s = binary add i64 %x, 1
    %b = cmp ne i64 %s, 0
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(11)
    return void

  block ^no:
    call void @note(12)
    return void
}

# The two compares of a select: x >= 0 && x < 100, and x == 3 || x == 4.
function @select_both(%x : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp ge i64 %x, 0
    %b = cmp lt i64 %x, 100
    %r = select i64 %a, %b, 0
    return i64 %r
}

function @select_either(%x : u32) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq u32 %x, 3
    %b = cmp eq u32 %x, 4
    %r = select i64 %a, 1, %b
    return i64 %r
}

# A select of one value and a select of the same value: one select.
function @nested(%x : i32, %k : ptr, %l : ptr) -> ptr [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary and i32 %x, 1
    %b = binary and i32 %x, 64
    %inner = select ptr %b, %k, %l
    %r = select ptr %a, %k, %inner
    return ptr %r
}

function @nested_both(%x : i16, %k : i64, %l : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp gt i16 %x, 2
    %b = cmp lt i16 %x, 9
    %inner = select i64 %b, %k, %l
    %r = select i64 %a, %inner, %l
    return i64 %r
}

# Both tests stay: two values, values that are no run and differ in two
# bits, and a shared target that tells the two edges apart. A second block
# whose value the next block reads keeps its test here; once the value has
# moved to its reader, the late run that joins tests (join_branch_tests)
# makes the two one run, `x - 5 <= 1`.
function @two_inputs(%x : i32, %y : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i32 %x, 3
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp eq i32 %y, 4
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(13)
    return void

  block ^no:
    call void @note(14)
    return void
}

function @apart(%x : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i32 %x, 3
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp eq i32 %x, 9
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(15)
    return void

  block ^no:
    call void @note(16)
    return void
}

function @value_read_later(%x : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i32 %x, 5
    branch %a, ^yes, ^more

  block ^more:
    %s = binary sub i32 %x, 6
    %b = cmp eq i32 %x, 6
    branch %b, ^yes, ^no

  block ^yes:
    call void @note(17)
    return void

  block ^no:
    %w = convert sext i64 i32 %s
    call void @note(%w)
    return void
}

function @edges_differ(%x : i32) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %a = cmp eq i32 %x, 5
    branch %a, ^yes, ^more

  block ^more:
    %b = cmp eq i32 %x, 6
    branch %b, ^yes, ^no

  block ^yes:
    %v = phi i64 [^entry: 18, ^more: 19]
    call void @note(%v)
    return void

  block ^no:
    call void @note(20)
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    call void @either_bit(16)
    call void @both_clear(3)
    call void @range(20)
    call void @run_of_three(7)
    call void @one_bit_apart(12)
    call void @neither_end(-1)
    call void @two_inputs(1, 4)
    call void @apart(9)
    call void @value_read_later(4)
    call void @edges_differ(6)
    %a = call i64 @select_both(99)
    %b = call i64 @select_either(5)
    %p = call ptr @nested(64, @seen, @spare)
    %c = load i64 %p
    %d = call i64 @nested_both(9, 7, 1)
    %e = binary add i64 %a, %b
    %f = binary add i64 %c, %d
    %g = binary add i64 %e, %f
    %r = convert trunc i32 i64 %g
    return i32 %r
}
