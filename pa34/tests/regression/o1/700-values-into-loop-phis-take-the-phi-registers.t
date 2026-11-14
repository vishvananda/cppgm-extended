# A value that a loop's back edge carries into a phi is computed in the phi's
# register, and the edge copies nothing. Each new value of @orbit is computed
# from the phis of other values: placed in a dying operand's register, the
# four results would land one register off and rotate on every iteration.
function @orbit(%cr : f64, %ci : f64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^head

  block ^head:
    %i = phi i64 [^entry: 0, ^body: %next]
    %zr = phi f64 [^entry: 0.0, ^body: %zr2]
    %zi = phi f64 [^entry: 0.0, ^body: %zi2]
    %tr = phi f64 [^entry: 0.0, ^body: %tr2]
    %ti = phi f64 [^entry: 0.0, ^body: %ti2]
    %norm = binary add f64 %ti, %tr
    %inside = cmp le f64 %norm, 4.0
    branch %inside, ^body, ^done

  block ^body:
    %twice = binary add f64 %zr, %zr
    %cross = binary mul f64 %twice, %zi
    %zi2 = binary add f64 %ci, %cross
    %diff = binary sub f64 %tr, %ti
    %zr2 = binary add f64 %cr, %diff
    %tr2 = binary mul f64 %zr2, %zr2
    %ti2 = binary mul f64 %zi2, %zi2
    %next = binary add nowrap i64 %i, 1
    %more = cmp lt i64 %next, 50
    branch %more, ^head, ^done

  block ^done:
    %count = phi i64 [^head: %i, ^body: %next]
    return i64 %count
}

function @main() -> i64 [role=entry] {
  block ^entry:
    %stays = call i64 @orbit(0.0, 0.0)
    %leaves = call i64 @orbit(2.0, 2.0)
    %later = call i64 @orbit(0.5, 0.5)
    %stays_bad = cmp ne i64 %stays, 50
    %leaves_bad = cmp ne i64 %leaves, 1
    %later_bad = cmp ne i64 %later, 5
    %bad0 = binary or i64 %stays_bad, %leaves_bad
    %bad = binary or i64 %bad0, %later_bad
    return i64 %bad
}
