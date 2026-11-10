# A jump to a short latch whose test the jumping block's values decide
# takes a copy of the latch. ^zero sets %x's next value to 1 and jumps to
# ^latch; on the way around, ^head's test of that value is decided, so the
# copy of the latch at the end of ^zero branches straight to ^body.

function @walk(%n : i64, %rounds : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head
  block ^head:
    %x = phi i64 [^entry: %n, ^latch: %v]
    %k = phi i64 [^entry: 0, ^latch: %k2]
    %acc = phi i64 [^entry: 0, ^latch: %acc2]
    %z = cmp eq i64 %x, 0
    branch %z, ^zero, ^body
  block ^body:
    %x1 = binary sub i64 %x, 1
    %a1 = binary add i64 %acc, %x
    jump ^latch
  block ^zero:
    %a0 = binary add i64 %acc, 100
    jump ^latch
  block ^latch:
    %v = phi i64 [^body: %x1, ^zero: 1]
    %acc2 = phi i64 [^body: %a1, ^zero: %a0]
    %k2 = binary add i64 %k, 1
    %more = cmp lt i64 %k2, %rounds
    branch %more, ^head, ^out
  block ^out:
    return i64 %acc2
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @walk(5, 40)
    %w = convert trunc i32 i64 %a
    %r = binary mod i32 %w, 251
    return i32 %r
}
