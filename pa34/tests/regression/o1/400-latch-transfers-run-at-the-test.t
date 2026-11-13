# A loop block that ends in a test and repeats the loop through an edge
# block of register transfers (here %i takes %half) runs the transfers at
# the test: the block then closes the loop with its own conditional branch,
# and the loop executes no unconditional jump. The compare reads the
# register the transfer writes, so the transfer follows it (a move leaves
# the flags as they are). Rotation does not copy the block into the jump
# that is left: @halvings is one block that branches to itself.
function @halvings(%n : i64) -> i64 [binding=internal, no_inline=yes] {
  block ^entry:
    jump ^loop

  block ^loop:
    %i = phi i64 [^entry: %n, ^loop: %half]
    %steps = phi i64 [^entry: 0, ^loop: %more]
    %half = binary ushr i64 %i, 1
    %more = binary add i64 %steps, 1
    %go = cmp ugt i64 %i, 1
    branch %go, ^loop, ^done

  block ^done:
    return i64 %more
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @halvings(40)
    %b = call i64 @halvings(1)
    %scaled = binary mul i64 %a, 10
    %sum = binary add i64 %scaled, %b
    %t = convert trunc i32 i64 %sum
    return i32 %t
}
