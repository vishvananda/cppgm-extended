# A select between the literals 1 and 0 is its condition's truth: the flags
# set the result's byte (`setcc`), with no conditional move. `@less` takes a
# compare's condition, `@not_less` the opposite one, `@nonzero` tests an
# integer and `@field` tests a loaded value in memory (`cmp [m], 0`).
function @less(%a : i32, %b : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp lt i32 %a, %b
    %r = select i32 %c, 1, 0
    return i32 %r
}

function @not_less(%a : i32, %b : i32) -> i32 [binding=strong, no_inline=yes] {
  block ^entry:
    %c = cmp lt i32 %a, %b
    %r = select i32 %c, 0, 1
    return i32 %r
}

function @nonzero(%a : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %r = select i64 %a, 1, 0
    return i64 %r
}

function @field(%p : ptr, %v : i64) -> i64 [binding=strong, no_inline=yes] {
  block ^entry:
    %a = load i64 %p
    %r = select i64 %a, %v, 9
    return i64 %r
}

function @main() -> i32 [role=entry] {
  slot $cell : i64
  block ^entry:
    %p = addr $cell
    store i64 0, %p
    %a = call i32 @less(1, 2)
    %b = call i32 @less(2, 2)
    %c = call i32 @not_less(1, 2)
    %d = call i32 @not_less(3, 2)
    %e = call i64 @nonzero(0)
    %f = call i64 @nonzero(-4)
    %g = call i64 @field(%p, 5)
    store i64 3, %p
    %h = call i64 @field(%p, 5)
    %ok0 = cmp eq i32 %a, 1
    %ok1 = cmp eq i32 %b, 0
    %ok2 = cmp eq i32 %c, 0
    %ok3 = cmp eq i32 %d, 1
    %ok4 = cmp eq i64 %e, 0
    %ok5 = cmp eq i64 %f, 1
    %ok6 = cmp eq i64 %g, 9
    %ok7 = cmp eq i64 %h, 5
    %a0 = binary and i64 %ok0, %ok1
    %a1 = binary and i64 %a0, %ok2
    %a2 = binary and i64 %a1, %ok3
    %a3 = binary and i64 %a2, %ok4
    %a4 = binary and i64 %a3, %ok5
    %a5 = binary and i64 %a4, %ok6
    %a6 = binary and i64 %a5, %ok7
    %r = convert trunc i32 i64 %a6
    %code = binary sub i32 1, %r
    return i32 %code
}
