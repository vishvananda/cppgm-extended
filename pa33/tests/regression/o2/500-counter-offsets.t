# A counter of an invariant step that only additions of invariants read starts
# at their sum, and the additions go.

function @rows(%base : ptr, %step : i32, %row : i32, %plane : i32, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %c = phi i32 [^entry: 0, ^body: %c1]
    %at = phi i32 [^entry: 0, ^body: %at1]
    %t = phi i64 [^entry: 0, ^body: %t1]
    %a = binary add nowrap i32 %at, %row
    %b = binary add nowrap i32 %a, %plane
    %p = index f64 %base, %b
    %v = load i64 %p
    %t1 = binary add i64 %t, %v
    %at1 = binary add nowrap i32 %at, %step
    %c1 = binary add nowrap i32 %c, 1
    %d = cmp lt i32 %c1, %n
    branch %d, ^body, ^done
  block ^done:
    return i64 %t1
}

# From a start that is not zero, and through a copy; the wrapping addition takes
# the counter's promise away.
function @wrapping(%base : ptr, %start : i32, %step : i32, %row : i32, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %c = phi i32 [^entry: 0, ^body: %c1]
    %at = phi i32 [^entry: %start, ^body: %at1]
    %t = phi i64 [^entry: 0, ^body: %t1]
    %m = copy i32 %at
    %a = binary add i32 %m, %row
    %p = index f64 %base, %a
    %v = load i64 %p
    %t1 = binary add i64 %t, %v
    %at1 = binary add nowrap i32 %at, %step
    %c1 = binary add nowrap i32 %c, 1
    %d = cmp lt i32 %c1, %n
    branch %d, ^body, ^done
  block ^done:
    return i64 %t1
}

# The addition is read after the loop: it stays a value of the counter.
function @read_after(%step : i64, %row : i64, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %c = phi i64 [^entry: 0, ^body: %c1]
    %at = phi i64 [^entry: 0, ^body: %at1]
    %a = binary add i64 %at, %row
    %at1 = binary add i64 %at, %step
    %c1 = binary add i64 %c, 1
    %d = cmp lt i64 %c1, %n
    branch %d, ^body, ^done
  block ^done:
    return i64 %a
}

# Only some passes reach the addition: the counter keeps wrapping arithmetic.
function @some_passes(%base : ptr, %step : i32, %row : i32, %n : i32) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %c = phi i32 [^entry: 0, ^tail: %c1]
    %at = phi i32 [^entry: 0, ^tail: %at1]
    %t = phi i64 [^entry: 0, ^tail: %t1]
    %odd = binary and i32 %c, 1
    %o = cmp ne i32 %odd, 0
    branch %o, ^read, ^tail
  block ^read:
    %a = binary add nowrap i32 %at, %row
    %p = index f64 %base, %a
    %v = load i64 %p
    %w = binary add i64 %t, %v
    jump ^tail
  block ^tail:
    %t1 = phi i64 [^body: %t, ^read: %w]
    %at1 = binary add nowrap i32 %at, %step
    %c1 = binary add nowrap i32 %c, 1
    %d = cmp lt i32 %c1, %n
    branch %d, ^body, ^done
  block ^done:
    return i64 %t1
}

# The counter is the loop's test: it keeps its value.
function @tested(%base : ptr, %row : i64, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %at = phi i64 [^entry: 0, ^body: %at1]
    %t = phi i64 [^entry: 0, ^body: %t1]
    %a = binary add i64 %at, %row
    %p = index f64 %base, %a
    %v = load i64 %p
    %t1 = binary add i64 %t, %v
    %at1 = binary add i64 %at, 3
    %d = cmp lt i64 %at1, %n
    branch %d, ^body, ^done
  block ^done:
    return i64 %t1
}

# The addition reads what the loop computes: the counter stays.
function @varying(%base : ptr, %step : i64, %n : i64) -> i64 [no_inline=yes] {
  block ^entry:
    jump ^body
  block ^body:
    %c = phi i64 [^entry: 0, ^body: %c1]
    %at = phi i64 [^entry: 0, ^body: %at1]
    %t = phi i64 [^entry: 0, ^body: %t1]
    %a = binary add i64 %at, %t
    %p = index f64 %base, %a
    %v = load i64 %p
    %t1 = binary add i64 %t, %v
    %at1 = binary add i64 %at, %step
    %c1 = binary add i64 %c, 1
    %d = cmp lt i64 %c1, %n
    branch %d, ^body, ^done
  block ^done:
    return i64 %t1
}

function @main() -> i32 [role=entry] {
  block ^entry:
    return i32 0
}
