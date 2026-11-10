# %k lives through the loop and feeds a phi of the web of %i: the rest of the web
# keeps one register, and only that edge moves a value.

global @table : obj<64x8> = { zero 64 }

function @walk(%n : i64, %k : i64) -> i64 [no_inline=yes] {
  block ^entry:
    %base = addr @table
    jump ^head
  block ^head:
    %i = phi i64 [^entry: %k, ^tail: %j]
    %t = phi i64 [^entry: 0, ^tail: %s]
    %p = index i64 %base, %i
    %v = load i64 %p
    %c = cmp lt i64 %v, %n
    branch %c, ^step, ^back
  block ^step:
    %a = binary add i64 %i, 1
    jump ^tail
  block ^back:
    jump ^tail
  block ^tail:
    %j = phi i64 [^step: %a, ^back: %k]
    %w = binary add i64 %v, 1
    %s = binary add i64 %t, %w
    %d = cmp lt i64 %s, %n
    branch %d, ^head, ^out
  block ^out:
    return i64 %s
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %r = call i64 @walk(4, 0)
    %v = convert trunc i32 i64 %r
    return i32 %v
}
