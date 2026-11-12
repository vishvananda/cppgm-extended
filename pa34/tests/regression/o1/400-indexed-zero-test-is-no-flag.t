# A value compared with zero straight from memory is usually zero when it is
# a field (a flag, an optional pointer), but an indexed element says
# nothing: `@count`'s test of `p[i]` takes the default of an equality, and
# in the loop the nonzero side follows the test, while `@flag`'s test of a
# field keeps its zero side after the test.
global @seen : i64 = 0

function @note(%v : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return void
}

function @count(%p : ptr, %n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^next: %j]
    %c = phi i64 [^entry: 0, ^next: %d]
    %a = index i64 %p, %i
    %v = load i64 %a
    %z = cmp eq i64 %v, 0
    branch %z, ^zero, ^nonzero

  block ^zero:
    call void @note(1)
    jump ^next

  block ^nonzero:
    call void @note(2)
    %c1 = binary add i64 %c, 1
    jump ^next

  block ^next:
    %d = phi i64 [^zero: %c, ^nonzero: %c1]
    %j = binary add i64 %i, 1
    %more = cmp lt i64 %j, %n
    branch %more, ^head, ^out

  block ^out:
    return i64 %d
}

function @flag(%p : ptr) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = binary add ptr %p, 8
    %v = load i64 %a
    %z = cmp eq i64 %v, 0
    branch %z, ^zero, ^nonzero

  block ^zero:
    call void @note(4)
    return i64 1

  block ^nonzero:
    call void @note(8)
    return i64 2
}

function @main() -> i32 [role=entry] {
  slot $data : obj<32x8>

  block ^entry:
    %buf = addr $data
    store i64 0, %buf
    %b1 = binary add ptr %buf, 8
    store i64 5, %b1
    %b2 = binary add ptr %buf, 16
    store i64 0, %b2
    %b3 = binary add ptr %buf, 24
    store i64 7, %b3
    %n = call i64 @count(%buf, 4)
    %f = call i64 @flag(%buf)
    %s = load i64 @seen
    %t1 = binary add i64 %s, %n
    %t2 = binary add i64 %t1, %f
    %t = convert trunc i32 i64 %t2
    return i32 %t
}
