# Each loop whose header only tests is rotated: the header stays as the guard
# and each latch ends in a copy of the test.

declare function @touch(%value : i64) -> void [unwind=no]

global @last : i64 = 0

# A count down: the phi takes the value the header computes for the next pass.
function @count_down(%n : i32) -> void [no_inline=yes] {
  block ^entry:
    jump ^header
  block ^header:
    %i = phi i32 [^entry: %n, ^body: %next]
    %next = binary sub nowrap i32 %i, 1
    branch %i, ^body, ^done
  block ^body:
    %v = load typed i64 @last
    %w = binary mul i64 %v, 3
    store typed i64 %w, @last
    jump ^header
  block ^done:
    return void
}

# Nothing to hoist or sink: the loop rotates all the same.
function @only_calls(%n : i64) -> void [no_inline=yes] {
  block ^entry:
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^body: %next]
    %c = cmp lt i64 %i, %n
    branch %c, ^body, ^done
  block ^body:
    call void @touch(%i)
    %next = binary add i64 %i, 1
    jump ^header
  block ^done:
    return void
}

# A guard of literals folds.
function @literal_guard(%p : ptr) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header
  block ^header:
    %i = phi i64 [^entry: 0, ^body: %next]
    %s = phi i64 [^entry: 0, ^body: %t]
    %c = cmp lt i64 %i, 1000
    branch %c, ^body, ^done
  block ^body:
    %q = index i64 %p, %i
    %v = load i64 %q
    %t = binary add i64 %s, %v
    %next = binary add i64 %i, 1
    jump ^header
  block ^done:
    return i64 %s
}

# A phi of a phi of the header stays unrotated: each is stored on the edge.
function @swap(%n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^header
  block ^header:
    %a = phi i64 [^entry: 1, ^body: %b]
    %b = phi i64 [^entry: 2, ^body: %a]
    %i = phi i64 [^entry: 0, ^body: %next]
    %c = cmp lt i64 %i, %n
    branch %c, ^body, ^done
  block ^body:
    call void @touch(%a)
    %next = binary add i64 %i, 1
    jump ^header
  block ^done:
    return i64 %b
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
