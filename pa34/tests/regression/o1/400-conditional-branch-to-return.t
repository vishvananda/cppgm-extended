# A block ending in a conditional branch to the next block and a jump to a
# block that only returns keeps the jump: the two become one inverted branch
# to the return, and the loop falls into its body (`jle ^done`), where a
# copy of the return would leave a taken branch to the next block
# (`jg ^body; ret`).
function @sum(%n : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: %n, ^body: %next]
    %s = phi i64 [^entry: 0, ^body: %t]
    %more = cmp gt i64 %i, 0
    branch %more, ^body, ^done

  block ^body:
    %t = binary add i64 %s, %i
    %next = binary sub i64 %i, 1
    jump ^head

  block ^done:
    return i64 %s
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %s = call i64 @sum(5)
    %z = call i64 @sum(0)
    %t1 = binary add i64 %s, %z
    %t = convert trunc i32 i64 %t1
    return i32 %t
}
