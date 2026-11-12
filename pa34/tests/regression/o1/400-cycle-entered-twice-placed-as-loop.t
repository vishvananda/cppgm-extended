# A cycle entered in two places (a and b, as jump threading leaves some)
# has no members and no exits. Placement still orders it as a loop (the
# block its walk found first weighs 8 times its inflow), and the path from a
# takes a copy of b's test; where blocks are weighed against the entry
# (copied tails, the frame's order), it weighs only its inflow, since with
# no exit every block after it would weigh as much. Here the cycle runs
# behind a compare with a constant (3 times in 10): at its inflow, the path
# from b into a's test runs less than a quarter as often as the entry and
# jumps to a's own test, which a hot tail would not.
global @seen : i64 = 0

function @note(%v : i64) -> void [binding=strong, no_inline=yes] {
  block ^entry:
    %old = load i64 @seen
    %new = binary add i64 %old, %v
    store i64 %new, @seen
    return void
}

# a and b form a cycle entered at either.
function @tangle(%n : i64, %odd : i64, %key : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %go = cmp eq i64 %key, 7
    branch %go, ^enter, ^skip

  block ^enter:
    branch %odd, ^b, ^a

  block ^a:
    %i = phi i64 [^enter: %n, ^b: %j]
    %small = cmp lt i64 %i, 3
    branch %small, ^rare, ^b

  block ^rare:
    call void @note(1)
    jump ^b

  block ^b:
    %k = phi i64 [^enter: %n, ^a: %i, ^rare: %i]
    %j = binary sub i64 %k, 1
    %more = cmp gt i64 %j, 0
    branch %more, ^a, ^done

  block ^done:
    return i64 %j

  block ^skip:
    return i64 %n
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %a = call i64 @tangle(5, 0, 7)
    %b = call i64 @tangle(6, 1, 7)
    %s = load i64 @seen
    %t1 = binary add i64 %s, %a
    %t2 = binary add i64 %t1, %b
    %t = convert trunc i32 i64 %t2
    return i32 %t
}
