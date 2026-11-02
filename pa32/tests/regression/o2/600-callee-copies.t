# A call that passes a defined function for a parameter its callee calls
# through calls a copy that calls the function directly, and the copy's
# recursive call passes it on to itself, so the comparison is inlined there
# (the copy's recursion then accumulates into a loop, inlined into main).
# The other caller passes a function it computes and keeps the original.
global @data : obj<32x8> = {
  i64 5
  i64 3
  i64 8
  i64 1
}

function @less(%a : i64, %b : i64) -> i64 [binding=internal] {
  block ^entry:
    %c = cmp lt i64 %a, %b
    return i64 %c
}

function @more(%a : i64, %b : i64) -> i64 [binding=internal] {
  block ^entry:
    %c = cmp gt i64 %a, %b
    return i64 %c
}

function @count(%p : ptr, %n : i64, %f : ptr) -> i64 [binding=strong] {
  block ^entry:
    %more = cmp gt i64 %n, 1
    branch %more, ^step, ^none

  block ^step:
    %a = load typed i64 %p
    %q = index i8 %p, 8
    %b = load typed i64 %q
    %x = call i64 %f(%a, %b) as (%l : i64, %r : i64) -> i64
    %m = binary sub i64 %n, 1
    %rest = call i64 @count(%q, %m, %f)
    %sum = binary add i64 %x, %rest
    return i64 %sum

  block ^none:
    return i64 0
}

function @main() -> i32 [role=entry] {
  block ^entry:
    %ascending = call i64 @count(@data, 4, @less)
    %pick = load volatile i64 @data
    %up = cmp gt i64 %pick, 4
    %which = select ptr %up, @more, @less
    %descending = call i64 @count(@data, 4, %which)
    %scaled = binary mul i64 %ascending, 10
    %all = binary add i64 %scaled, %descending
    %r = convert trunc i32 i64 %all
    return i32 %r
}
