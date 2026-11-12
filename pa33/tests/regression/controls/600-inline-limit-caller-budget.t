# From O2, a callee that adds little goes inline past its caller's growth
# budget: its call costs about as much as its copy. With the budget spent
# (the control runs at -O2 with `--inline-limit caller-budget=1`), @caller
# keeps its calls to @work and @medium and still takes @small. Observable
# writes keep each callee's work.
global @sink : i64 = 0

function @work(%value : i64) -> i64 [binding=strong] {
  block ^entry:
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    return i64 %value
}

function @medium(%value : i64) -> i64 [binding=strong] {
  block ^entry:
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    return i64 %value
}

function @small(%value : i64) -> i64 [binding=strong] {
  block ^entry:
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    store volatile i64 %value, @sink
    return i64 %value
}

function @caller(%value : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = call i64 @work(%value)
    %b = call i64 @small(%a)
    %c = call i64 @medium(%b)
    return i64 %c
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %r = call i64 @caller(0)
    %t = convert trunc i32 i64 %r
    return i32 %t
}
