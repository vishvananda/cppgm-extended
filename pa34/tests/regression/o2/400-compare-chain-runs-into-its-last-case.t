# A switch lowered to a compare chain is one block: a branch per case, then
# a jump to the default. Only its end runs into the block laid out after
# it, so the walk follows one of the last test's two targets (an earlier
# case's branch reaches its target wherever that lies): the heavier one
# that is free. Each branch takes its share of what the tests before it
# left (a compare with a constant 3 times in 10, a range of a few adjacent
# constants the same), so the default weighs most here; it is the loop's
# shared bottom, which the first case enters about as often, and waits for
# the cases. The last case then follows the chain, and the chain ends in
# one conditional branch to the default instead of a branch and a jump.
global @kinds : obj<8x1> [binding=internal] = {
  u8 4
  u8 7
  u8 25
  u8 33
  u8 9
  u8 32
  u8 1
  u8 4
}
global @seen : i64 = 0

function @count(%bytes : ptr, %n : i64) -> void [binding=internal, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^next: %i1]
    %more = cmp lt i64 %i, %n
    branch %more, ^body, ^done

  block ^body:
    %at = index u8 %bytes, %i
    %kind = load u8 %at
    switch %kind, ^next, 4:^four, 25:^quarter, 32:^pair, 33:^pair

  block ^four:
    %a = load i64 @seen
    %a1 = binary add i64 %a, 1
    store i64 %a1, @seen
    jump ^next

  block ^quarter:
    %b = load i64 @seen
    %b1 = binary add i64 %b, 10
    store i64 %b1, @seen
    jump ^next

  block ^pair:
    %c = load i64 @seen
    %c1 = binary add i64 %c, 40
    store i64 %c1, @seen
    jump ^next

  block ^next:
    %i1 = binary add i64 %i, 1
    jump ^head

  block ^done:
    return void
}

function @main() -> i32 [role=entry] {
  block ^entry:
    call void @count(@kinds, 8)
    %s = load i64 @seen
    %t = convert trunc i32 i64 %s
    return i32 %t
}
